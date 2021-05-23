// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FFmpeg.h>

#include <tlrCore/Assert.h>
#include <tlrCore/String.h>

extern "C"
{
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>

} // extern "C"

#include <iostream>
#include <sstream>

namespace tlr
{
    namespace ffmpeg
    {
        std::string getErrorLabel(int r)
        {
            char buf[string::cBufferSize];
            av_strerror(r, buf, string::cBufferSize);
            return std::string(buf);
        }
            
        void Read::_init(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            IRead::_init(fileName, defaultSpeed);
            _videoFrameCache.setMax(1);
            _running = true;
            _thread = std::thread(
                [this, fileName]
                {
                    try
                    {
                        _open(fileName);
                        _run();
                    }
                    catch (const std::exception& e)
                    {
                        //std::cout << "ERROR: " << e.what() << std::endl;
                        //! \todo How should this be handled?
                        _infoPromise.set_value(io::Info());
                    }
                    _close();
                });
        }

        Read::Read()
        {}

        Read::~Read()
        {
            _running = false;
            if (_thread.joinable())
            {
                _thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(fileName, defaultSpeed);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            return _infoPromise.get_future();
        }

        std::future<io::VideoFrame> Read::getVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            VideoFrameRequest request;
            request.time = time;
            request.image = image;
            auto future = request.promise.get_future();
            {
                std::unique_lock<std::mutex> lock(_requestMutex);
                _videoFrameRequests.push_back(std::move(request));
            }
            _requestCV.notify_one();
            return future;
        }

        bool Read::hasVideoFrames()
        {
            std::unique_lock<std::mutex> lock(_requestMutex);
            return !_videoFrameRequests.empty();
        }

        void Read::cancelVideoFrames()
        {
            std::unique_lock<std::mutex> lock(_requestMutex);
            _videoFrameRequests.clear();
        }

        void Read::_open(const std::string& fileName)
        {
            av_log_set_level(AV_LOG_QUIET);

            int r = avformat_open_input(
                &_avFormatContext,
                fileName.c_str(),
                nullptr,
                nullptr);
            if (r < 0)
            {
                std::stringstream ss;
                ss << fileName << ": " << getErrorLabel(r);
                throw std::runtime_error(ss.str());
            }
            r = avformat_find_stream_info(_avFormatContext, 0);
            if (r < 0)
            {
                std::stringstream ss;
                ss << fileName << ": " << getErrorLabel(r);
                throw std::runtime_error(ss.str());
            }
            //av_dump_format(_avFormatContext, 0, fileName.c_str(), 0);

            for (unsigned int i = 0; i < _avFormatContext->nb_streams; ++i)
            {
                if (-1 == _avVideoStream && _avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    _avVideoStream = i;
                }
            }
            if (-1 == _avVideoStream)
            {
                std::stringstream ss;
                ss << fileName << ": " << "No video stream found";
                throw std::runtime_error(ss.str());
            }

            _avFrame = av_frame_alloc();

            std::size_t sequenceSize = 0;
            if (_avVideoStream != -1)
            {
                auto avVideoStream = _avFormatContext->streams[_avVideoStream];
                auto avVideoCodecParameters = avVideoStream->codecpar;
                auto avVideoCodec = avcodec_find_decoder(avVideoCodecParameters->codec_id);
                if (!avVideoCodec)
                {
                    std::stringstream ss;
                    ss << fileName << ": " << "No video codec found";
                    throw std::runtime_error(ss.str());
                }
                _avCodecParameters[_avVideoStream] = avcodec_parameters_alloc();
                r = avcodec_parameters_copy(_avCodecParameters[_avVideoStream], avVideoCodecParameters);
                if (r < 0)
                {
                    std::stringstream ss;
                    ss << fileName << ": " << getErrorLabel(r);
                    throw std::runtime_error(ss.str());
                }
                _avCodecContext[_avVideoStream] = avcodec_alloc_context3(avVideoCodec);
                r = avcodec_parameters_to_context(_avCodecContext[_avVideoStream], _avCodecParameters[_avVideoStream]);
                if (r < 0)
                {
                    std::stringstream ss;
                    ss << fileName << ": " << getErrorLabel(r);
                    throw std::runtime_error(ss.str());
                }
                _avCodecContext[_avVideoStream]->thread_count = threadCount;
                _avCodecContext[_avVideoStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(_avCodecContext[_avVideoStream], avVideoCodec, 0);
                if (r < 0)
                {
                    std::stringstream ss;
                    ss << fileName << ": " << getErrorLabel(r);
                    throw std::runtime_error(ss.str());
                }

                io::VideoInfo videoInfo;
                videoInfo.info.size.w = _avCodecParameters[_avVideoStream]->width;
                videoInfo.info.size.h = _avCodecParameters[_avVideoStream]->height;
                videoInfo.codec = avVideoCodec->long_name;

                const AVPixelFormat avPixelFormat = static_cast<AVPixelFormat>(_avCodecParameters[_avVideoStream]->format);
                switch (avPixelFormat)
                {
                case AV_PIX_FMT_YUV420P:
                    videoInfo.info.pixelType = imaging::PixelType::YUV_420P;
                    break;
                case AV_PIX_FMT_RGB24:
                    videoInfo.info.pixelType = imaging::PixelType::RGB_U8;
                    break;
                case AV_PIX_FMT_GRAY8:
                    videoInfo.info.pixelType = imaging::PixelType::L_U8;
                    break;
                case AV_PIX_FMT_RGBA:
                    videoInfo.info.pixelType = imaging::PixelType::RGBA_U8;
                    break;
                default:
                    videoInfo.info.pixelType = imaging::PixelType::YUV_420P;
                    _avFrameRgb = av_frame_alloc();
                    _swsContext = sws_getContext(
                        _avCodecParameters[_avVideoStream]->width,
                        _avCodecParameters[_avVideoStream]->height,
                        avPixelFormat,
                        _avCodecParameters[_avVideoStream]->width,
                        _avCodecParameters[_avVideoStream]->height,
                        AV_PIX_FMT_YUV420P,
                        swsScaleFlags,
                        0,
                        0,
                        0);
                    break;
                }

                if (avVideoStream->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = avVideoStream->r_frame_rate.den;
                    r.den = avVideoStream->r_frame_rate.num;
                    sequenceSize = av_rescale_q(
                        avVideoStream->duration,
                        avVideoStream->time_base,
                        r);
                }
                else if (_avFormatContext->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = avVideoStream->r_frame_rate.den;
                    r.den = avVideoStream->r_frame_rate.num;
                    sequenceSize = av_rescale_q(
                        _avFormatContext->duration,
                        av_get_time_base_q(),
                        r);
                }
                videoInfo.duration = otime::RationalTime(
                    sequenceSize,
                    avVideoStream->r_frame_rate.num / double(avVideoStream->r_frame_rate.den));
                _info.video.push_back(videoInfo);

                _currentTime = otime::RationalTime(0, videoInfo.duration.rate());
            }

            AVDictionaryEntry* tag = nullptr;
            while ((tag = av_dict_get(_avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
            {
                _info.tags[tag->key] = tag->value;
            }

            _infoPromise.set_value(_info);
        }

        void Read::_run()
        {
            while (_running)
            {
                VideoFrameRequest request;
                bool requestValid = false;
                {
                    std::unique_lock<std::mutex> lock(_requestMutex);
                    _requestCV.wait_for(
                        lock,
                        requestTimeout,
                        [this]
                        {
                            return !_videoFrameRequests.empty();
                        });
                    if (!_videoFrameRequests.empty())
                    {
                        request.time = _videoFrameRequests.front().time;
                        request.image = std::move(_videoFrameRequests.front().image);
                        request.promise = std::move(_videoFrameRequests.front().promise);
                        _videoFrameRequests.pop_front();
                        requestValid = true;
                    }
                }
                if (requestValid)
                {
                    //std::cout << "request: " << request.time << std::endl;
                    io::VideoFrame videoFrame;
                    if (_videoFrameCache.get(request.time, videoFrame))
                    {
                        request.promise.set_value(videoFrame);
                    }
                    else
                    {
                        if (request.time != _currentTime)
                        {
                            //std::cout << "seek: " << request.time << std::endl;
                            _currentTime = request.time;

                            int64_t t = 0;
                            int stream = -1;
                            if (_avVideoStream != -1)
                            {
                                stream = _avVideoStream;
                                AVRational r;
                                r.num = _avFormatContext->streams[_avVideoStream]->r_frame_rate.den;
                                r.den = _avFormatContext->streams[_avVideoStream]->r_frame_rate.num;
                                t = av_rescale_q(
                                    request.time.value(),
                                    r,
                                    _avFormatContext->streams[_avVideoStream]->time_base);
                            }
                            if (_avVideoStream != -1)
                            {
                                avcodec_flush_buffers(_avCodecContext[_avVideoStream]);
                            }
                            if (av_seek_frame(
                                _avFormatContext,
                                stream,
                                t,
                                AVSEEK_FLAG_BACKWARD) < 0)
                            {
                                //! \todo How should this be handled?
                            }
                        }

                        if (request.image && request.image->getInfo() == _info.video[0].info)
                        {
                            videoFrame.image = request.image;
                        }
                        else
                        {
                            videoFrame.image = imaging::Image::create(_info.video[0].info);
                        }

                        int decoding = 0;
                        AVPacket packet;
                        while (0 == decoding || videoFrame.time < request.time)
                        {
                            if (av_read_frame(_avFormatContext, &packet) < 0)
                            {
                                if (_avVideoStream != -1)
                                {
                                    //! \todo How should this be handled?
                                }
                                break;
                            }
                            if (_avVideoStream == packet.stream_index)
                            {
                                decoding = _decodeVideo(packet, videoFrame, true, request.time);
                                if (decoding < 0)
                                {
                                    //! \todo How should this be handled?
                                    break;
                                }
                            }
                            av_packet_unref(&packet);
                        }

                        request.promise.set_value(videoFrame);
                        _videoFrameCache.add(request.time, videoFrame);
                        _currentTime = request.time + otime::RationalTime(1, _currentTime.rate());
                    }
                }
            }
        }

        void Read::_close()
        {
            if (_swsContext)
            {
                sws_freeContext(_swsContext);
            }
            if (_avFrameRgb)
            {
                av_frame_free(&_avFrameRgb);
            }
            if (_avFrame)
            {
                av_frame_free(&_avFrame);
            }
            for (auto i : _avCodecContext)
            {
                avcodec_close(i.second);
                avcodec_free_context(&i.second);
            }
            for (auto i : _avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (_avFormatContext)
            {
                avformat_close_input(&_avFormatContext);
            }
        }

        int Read::_decodeVideo(
            AVPacket& packet,
            io::VideoFrame& frame,
            bool hasSeek,
            const otime::RationalTime& seek)
        {
            int out = avcodec_send_packet(_avCodecContext[_avVideoStream], &packet);
            if (out < 0)
            {
                return out;
            }

            out = avcodec_receive_frame(_avCodecContext[_avVideoStream], _avFrame);
            if (out < 0)
            {
                return (out == AVERROR(EAGAIN) || out == AVERROR_EOF) ? 0 : out;
            }

            AVRational r;
            r.num = _avFormatContext->streams[_avVideoStream]->r_frame_rate.den;
            r.den = _avFormatContext->streams[_avVideoStream]->r_frame_rate.num;
            const auto& videoInfo = _info.video[0];
            frame.time = otime::RationalTime(
                av_rescale_q(
                    _avFrame->pts,
                    _avFormatContext->streams[_avVideoStream]->time_base,
                    r),
                videoInfo.duration.rate());
            if (hasSeek && frame.time < seek)
            {
                return 0;
            }
            frame.time = _currentTime;
            //std::cout << "frame: " << frame.time << std::endl;

            _copyVideo(frame.image);

            return 1;
        }

        void Read::_copyVideo(const std::shared_ptr<imaging::Image>& image)
        {
            const auto& info = image->getInfo();
            const std::size_t w = info.size.w;
            const std::size_t h = info.size.h;
            const AVPixelFormat avPixelFormat = static_cast<AVPixelFormat>(_avCodecParameters[_avVideoStream]->format);
            switch (avPixelFormat)
            {
            case AV_PIX_FMT_YUV420P:
            {
                const std::size_t w2 = w / 2;
                const std::size_t h2 = h / 2;
                for (std::size_t i = 0; i < h; ++i)
                {
                    memcpy(
                        image->getData() + w * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * i,
                        w);
                }
                for (std::size_t i = 0; i < h2; ++i)
                {
                    memcpy(
                        image->getData() + (w * h) + w2 * i,
                        _avFrame->data[1] + _avFrame->linesize[1] * i,
                        w2);
                    memcpy(
                        image->getData() + (w * h) + (w2 * h2) + w2 * i,
                        _avFrame->data[2] + _avFrame->linesize[2] * i,
                        w2);
                }
                break;
            }
            case AV_PIX_FMT_RGB24:
                for (std::size_t i = 0; i < h; ++i)
                {
                    memcpy(
                        image->getData() + w * 3 * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * 3 * i,
                        w * 3);
                }
                break;
            case AV_PIX_FMT_GRAY8:
                for (std::size_t i = 0; i < h; ++i)
                {
                    memcpy(
                        image->getData() + w * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * i,
                        w);
                }
                break;
            case AV_PIX_FMT_RGBA:
                for (std::size_t i = 0; i < h; ++i)
                {
                    memcpy(
                        image->getData() + w * 4 * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * 4 * i,
                        w * 4);
                }
                break;
            default:
                av_image_fill_arrays(
                    _avFrameRgb->data,
                    _avFrameRgb->linesize,
                    image->getData(),
                    AV_PIX_FMT_YUV420P,
                    w,
                    h,
                    1);
                sws_scale(
                    _swsContext,
                    (uint8_t const* const*)_avFrame->data,
                    _avFrame->linesize,
                    0,
                    _avCodecParameters[_avVideoStream]->height,
                    _avFrameRgb->data,
                    _avFrameRgb->linesize);
                break;
            }
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init({ ".mov", ".m4v", ".mp4", ".y4m", ".mkv" });
            return out;
        }

        /*bool Plugin::canRead(const std::string& fileName)
        {
            AVFormatContext* avFormatContext = nullptr;
            int r = avformat_open_input(
                &avFormatContext,
                fileName.c_str(),
                nullptr,
                nullptr);
            bool out = r >= 0;
            if (avFormatContext)
            {
                avformat_close_input(&avFormatContext);
            }
            return out;
        }*/

        std::shared_ptr<io::IRead> Plugin::read(
            const std::string& fileName,
            const otime::RationalTime& defaultSpeed)
        {
            return Read::create(fileName, defaultSpeed);
        }
    }
}
