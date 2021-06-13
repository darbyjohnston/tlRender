// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FFmpeg.h>

#include <tlrCore/Assert.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

extern "C"
{
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>

} // extern "C"

#include <cstring>

namespace tlr
{
    namespace ffmpeg
    {
        void Read::_init(
            const std::string& fileName,
            const io::Options& options)
        {
            IRead::_init(fileName, options);
            _running = true;
            _stopped = false;
            _thread = std::thread(
                [this, fileName]
                {
                    try
                    {
                        _open(fileName);
                        try
                        {
                            _run();
                        }
                        catch (const std::exception& e)
                        {}
                    }
                    catch (const std::exception& e)
                    {
                        _infoPromise.set_value(io::Info());
                    }
                    _stopped = true;
                    std::list<VideoFrameRequest> videoFrameRequests;
                    {
                        std::unique_lock<std::mutex> lock(_requestMutex);
                        videoFrameRequests.swap(_videoFrameRequests);
                    }
                    for (auto& i : videoFrameRequests)
                    {
                        i.promise.set_value(io::VideoFrame());
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
            const io::Options& options)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(fileName, options);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            return _infoPromise.get_future();
        }

        std::future<io::VideoFrame> Read::readVideoFrame(const otime::RationalTime& time)
        {
            VideoFrameRequest request;
            request.time = time;
            auto future = request.promise.get_future();
            if (!_stopped)
            {
                {
                    std::unique_lock<std::mutex> lock(_requestMutex);
                    _videoFrameRequests.push_back(std::move(request));
                }
                _requestCV.notify_one();
            }
            else
            {
                request.promise.set_value(io::VideoFrame());
            }
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

        void Read::stop()
        {
            _running = false;
        }

        bool Read::hasStopped() const
        {
            return _stopped;
        }

        void Read::_open(const std::string& fileName)
        {
            int r = avformat_open_input(
                &_avFormatContext,
                fileName.c_str(),
                nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            r = avformat_find_stream_info(_avFormatContext, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
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
                throw std::runtime_error(string::Format("{0}: No video stream found").arg(fileName));
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
                    throw std::runtime_error(string::Format("{0}: No video codec found").arg(fileName));
                }
                _avCodecParameters[_avVideoStream] = avcodec_parameters_alloc();
                r = avcodec_parameters_copy(_avCodecParameters[_avVideoStream], avVideoCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                _avCodecContext[_avVideoStream] = avcodec_alloc_context3(avVideoCodec);
                r = avcodec_parameters_to_context(_avCodecContext[_avVideoStream], _avCodecParameters[_avVideoStream]);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                _avCodecContext[_avVideoStream]->thread_count = threadCount;
                _avCodecContext[_avVideoStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(_avCodecContext[_avVideoStream], avVideoCodec, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }

                io::VideoInfo videoInfo;
                videoInfo.info.size.w = _avCodecParameters[_avVideoStream]->width;
                videoInfo.info.size.h = _avCodecParameters[_avVideoStream]->height;

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
                    _avFrame2 = av_frame_alloc();
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
                    sequenceSize = av_rescale_q(
                        avVideoStream->duration,
                        avVideoStream->time_base,
                        swap(avVideoStream->r_frame_rate));
                }
                else if (_avFormatContext->duration != AV_NOPTS_VALUE)
                {
                    sequenceSize = av_rescale_q(
                        _avFormatContext->duration,
                        av_get_time_base_q(),
                        swap(avVideoStream->r_frame_rate));
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
                        request.promise = std::move(_videoFrameRequests.front().promise);
                        _videoFrameRequests.pop_front();
                        requestValid = true;
                    }
                }
                if (requestValid)
                {
                    //std::cout << "request: " << request.time << std::endl;
                    io::VideoFrame videoFrame;

                    if (request.time != _currentTime)
                    {
                        //std::cout << "seek: " << request.time << std::endl;
                        _currentTime = request.time;
                        _imageBuffer.clear();
                        int64_t t = 0;
                        int stream = -1;
                        if (_avVideoStream != -1)
                        {
                            avcodec_flush_buffers(_avCodecContext[_avVideoStream]);
                            stream = _avVideoStream;
                            t = av_rescale_q(
                                request.time.value(),
                                swap(_avFormatContext->streams[_avVideoStream]->r_frame_rate),
                                _avFormatContext->streams[_avVideoStream]->time_base);
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

                    if (_imageBuffer.empty())
                    {
                        int decoding = 0;
                        AVPacket packet;
                        AVPacket* packetP = &packet;
                        while (0 == decoding)
                        {
                            if (packetP)
                            {
                                decoding = av_read_frame(_avFormatContext, packetP);
                                if (AVERROR_EOF == decoding)
                                {
                                    //avcodec_flush_buffers(_avCodecContext[_avVideoStream]);
                                    decoding = 0;
                                    packetP = nullptr;
                                }
                                else if (decoding < 0)
                                {
                                    //! \todo How should this be handled?
                                    break;
                                }
                            }
                            if (_avVideoStream == packet.stream_index)
                            {
                                decoding = avcodec_send_packet(_avCodecContext[_avVideoStream], packetP);
                                if (AVERROR_EOF == decoding)
                                {
                                    //! \todo How should this be handled?
                                    decoding = 0;
                                }
                                else if (decoding < 0)
                                {
                                    break;
                                }
                                decoding = _decodeVideo(packetP, request.time);
                                if (AVERROR(EAGAIN) == decoding || AVERROR_EOF == decoding)
                                {
                                    decoding = 0;
                                }
                                else if (decoding < 0)
                                {
                                    //! \todo How should this be handled?
                                    break;
                                }
                            }
                            if (packetP)
                            {
                                av_packet_unref(packetP);
                            }
                        }
                    }

                    if (!_imageBuffer.empty())
                    {
                        videoFrame.time = request.time;
                        videoFrame.image = *_imageBuffer.begin();
                        _imageBuffer.pop_front();
                    }

                    request.promise.set_value(videoFrame);
                    _currentTime = request.time + otime::RationalTime(1.0, _currentTime.rate());
                }
            }
        }

        void Read::_close()
        {
            if (_swsContext)
            {
                sws_freeContext(_swsContext);
            }
            if (_avFrame2)
            {
                av_frame_free(&_avFrame2);
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

        int Read::_decodeVideo(AVPacket* packet, const otime::RationalTime& seek)
        {
            int out = 0;
            while (0 == out)
            {
                out = avcodec_receive_frame(_avCodecContext[_avVideoStream], _avFrame);
                if (out < 0)
                {
                    return out;
                }

                const auto& videoInfo = _info.video[0];
                const auto t = otime::RationalTime(
                    av_rescale_q(
                        _avFrame->pts,
                        _avFormatContext->streams[_avVideoStream]->time_base,
                        swap(_avFormatContext->streams[_avVideoStream]->r_frame_rate)),
                    videoInfo.duration.rate());
                if (t >= seek)
                {
                    //std::cout << "frame: " << t << std::endl;
                    auto image = imaging::Image::create(videoInfo.info);
                    _copyVideo(image);
                    _imageBuffer.push_back(image);
                    out = 1;
                }
            }
            return out;
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
                    std::memcpy(
                        image->getData() + w * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * i,
                        w);
                }
                for (std::size_t i = 0; i < h2; ++i)
                {
                    std::memcpy(
                        image->getData() + (w * h) + w2 * i,
                        _avFrame->data[1] + _avFrame->linesize[1] * i,
                        w2);
                    std::memcpy(
                        image->getData() + (w * h) + (w2 * h2) + w2 * i,
                        _avFrame->data[2] + _avFrame->linesize[2] * i,
                        w2);
                }
                break;
            }
            case AV_PIX_FMT_RGB24:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        image->getData() + w * 3 * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * 3 * i,
                        w * 3);
                }
                break;
            case AV_PIX_FMT_GRAY8:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        image->getData() + w * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * i,
                        w);
                }
                break;
            case AV_PIX_FMT_RGBA:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        image->getData() + w * 4 * i,
                        _avFrame->data[0] + _avFrame->linesize[0] * 4 * i,
                        w * 4);
                }
                break;
            default:
                av_image_fill_arrays(
                    _avFrame2->data,
                    _avFrame2->linesize,
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
                    _avFrame2->data,
                    _avFrame2->linesize);
                break;
            }
        }
    }
}
