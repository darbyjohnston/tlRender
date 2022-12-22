// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/opt.h>

} // extern "C"

namespace tl
{
    namespace ffmpeg
    {
        AVIOBufferData::AVIOBufferData()
        {}

        AVIOBufferData::AVIOBufferData(const uint8_t* p, size_t size) :
            p(p),
            pCurrent(p),
            size(size)
        {}

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            const size_t remaining = (bufferData->p + bufferData->size) - bufferData->pCurrent;
            bufSize = std::min(static_cast<size_t>(bufSize), remaining);
            if (!bufSize)
            {
                return AVERROR_EOF;
            }

            memcpy(buf, bufferData->pCurrent, bufSize);
            bufferData->pCurrent += bufSize;

            return bufSize;
        }

        int64_t avIOBufferSeek(void* opaque, int64_t offset, int whence)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            if (whence & AVSEEK_SIZE)
            {
                return bufferData->size;
            }

            bufferData->pCurrent = bufferData->p + offset;

            return offset;
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);

            TLRENDER_P();

            auto i = options.find("ffmpeg/YUVToRGBConversion");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.yuvToRGBConversion;
            }
            i = options.find("ffmpeg/AudioChannelCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                size_t channelCount = 0;
                ss >> channelCount;
                p.audioConvertInfo.channelCount = std::min(channelCount, static_cast<size_t>(255));
            }
            i = options.find("ffmpeg/AudioDataType");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.audioConvertInfo.dataType;
            }
            i = options.find("ffmpeg/AudioSampleRate");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.audioConvertInfo.sampleRate;
            }
            i = options.find("ffmpeg/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.threadCount;
            }
            i = options.find("ffmpeg/RequestTimeout");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.requestTimeout;
            }
            i = options.find("ffmpeg/VideoBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.videoBufferSize;
            }
            i = options.find("ffmpeg/AudioBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.audioBufferSize;
            }

            p.running = true;
            p.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        _openVideo(path.get());
                        _openAudio(path.get());
                        p.infoPromise.set_value(p.info);

                        try
                        {
                            _run();
                        }
                        catch (const std::exception& e)
                        {
                            if (auto logSystem = _logSystem.lock())
                            {
                                const std::string id = string::Format("tl::io::ffmpeg::Read ({0}: {1})").
                                    arg(__FILE__).
                                    arg(__LINE__);
                                logSystem->print(id, string::Format("{0}: {1}").
                                    arg(_path.get()).
                                    arg(e.what()),
                                    log::Type::Error);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = string::Format("tl::io::ffmpeg::Read ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, string::Format("{0}: {1}").
                                arg(_path.get()).
                                arg(e.what()),
                                log::Type::Error);
                        }
                        p.infoPromise.set_value(io::Info());
                    }

                    {
                        std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
                        std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            p.stopped = true;
                            videoRequests = std::move(p.videoRequests);
                            if (p.currentVideoRequest)
                            {
                                videoRequests.push_front(p.currentVideoRequest);
                                p.currentAudioRequest.reset();
                            }
                            audioRequests = std::move(p.audioRequests);
                            if (p.currentAudioRequest)
                            {
                                audioRequests.push_front(p.currentAudioRequest);
                                p.currentAudioRequest.reset();
                            }
                        }
                        for (auto& request: videoRequests)
                        {
                            request->promise.set_value(io::VideoData());
                        }
                        for (auto& request : audioRequests)
                        {
                            request->promise.set_value(io::AudioData());
                        }
                    }

                    _close();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            return _p->infoPromise.get_future();
        }

        std::future<io::VideoData> Read::readVideo(
            const otime::RationalTime& time,
            uint16_t)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        std::future<io::AudioData> Read::readAudio(const otime::TimeRange& time)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        void Read::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                videoRequests = std::move(p.videoRequests);
                if (p.currentVideoRequest)
                {
                    videoRequests.push_front(p.currentVideoRequest);
                    p.currentVideoRequest.reset();
                }
                audioRequests = std::move(p.audioRequests);
                if (p.currentAudioRequest)
                {
                    audioRequests.push_front(p.currentAudioRequest);
                    p.currentAudioRequest.reset();
                }
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(io::VideoData());
            }
            for (auto& request : audioRequests)
            {
                request->promise.set_value(io::AudioData());
            }
        }

        void Read::stop()
        {
            _p->running = false;
        }

        void Read::_run()
        {
            TLRENDER_P();

            if (p.video.avStream != -1)
            {
                p.video.avFrame = av_frame_alloc();
                if (!p.video.avFrame)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(_path.get()));
                }

                switch (p.video.avInputPixelFormat)
                {
                case AV_PIX_FMT_RGB24:
                case AV_PIX_FMT_GRAY8:
                case AV_PIX_FMT_RGBA:
                case AV_PIX_FMT_YUV420P:
                    break;
                default:
                {
                    p.video.avFrame2 = av_frame_alloc();
                    if (!p.video.avFrame2)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(_path.get()));
                    }

                    /*p.video.swsContext = sws_getContext(
                        p.video.avCodecParameters[p.video.avStream]->width,
                        p.video.avCodecParameters[p.video.avStream]->height,
                        p.video.avInputPixelFormat,
                        p.video.avCodecParameters[p.video.avStream]->width,
                        p.video.avCodecParameters[p.video.avStream]->height,
                        p.video.avOutputPixelFormat,
                        swsScaleFlags,
                        0,
                        0,
                        0);
                    if (!p.video.swsContext)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot get context").arg(_path.get()));
                    }*/
                    p.video.swsContext = sws_alloc_context();
                    if (!p.video.swsContext)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(_path.get()));
                    }
                    av_opt_set_defaults(p.video.swsContext);
                    int r = av_opt_set_int(p.video.swsContext, "srcw", p.video.avCodecParameters[p.video.avStream]->width, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(p.video.swsContext, "srch", p.video.avCodecParameters[p.video.avStream]->height, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(p.video.swsContext, "src_format", p.video.avInputPixelFormat, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(p.video.swsContext, "dstw", p.video.avCodecParameters[p.video.avStream]->width, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(p.video.swsContext, "dsth", p.video.avCodecParameters[p.video.avStream]->height, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(p.video.swsContext, "dst_format", p.video.avOutputPixelFormat, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(p.video.swsContext, "sws_flags", swsScaleFlags, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(p.video.swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
                    r = sws_init_context(p.video.swsContext, nullptr, nullptr);
                    if (r < 0)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot initialize sws context").arg(_path.get()));
                    }
                    break;
                }
                }
            }

            if (p.audio.avStream != -1)
            {
                p.audio.avFrame = av_frame_alloc();
                if (!p.audio.avFrame)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(_path.get()));
                }

                AVChannelLayout channelLayout;
                av_channel_layout_default(&channelLayout, p.info.audio.channelCount);
                const auto& avCodecParameters = p.audio.avCodecParameters[p.audio.avStream];
                int r = swr_alloc_set_opts2(
                    &p.audio.swrContext,
                    &channelLayout,
                    fromAudioType(p.info.audio.dataType),
                    p.info.audio.sampleRate,
                    &avCodecParameters->ch_layout,
                    static_cast<AVSampleFormat>(avCodecParameters->format),
                    avCodecParameters->sample_rate,
                    0,
                    NULL);
                av_channel_layout_uninit(&channelLayout);
                if (!p.audio.swrContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot get context").arg(_path.get()));
                }
                swr_init(p.audio.swrContext);
            }

            p.logTimer = std::chrono::steady_clock::now();
            while (p.running)
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(p.requestTimeout),
                        [this]
                        {
                            return
                                !_p->videoRequests.empty() ||
                                _p->currentVideoRequest ||
                                !_p->audioRequests.empty() ||
                                _p->currentAudioRequest ||
                                _p->video.buffer.size() < _p->videoBufferSize ||
                                audio::getSampleCount(_p->audio.buffer) <
                                    _p->audioBufferSize.rescaled_to(_p->info.audio.sampleRate).value();
                        }))
                    {
                        if (!p.currentVideoRequest && !p.videoRequests.empty())
                        {
                            p.currentVideoRequest = p.videoRequests.front();
                            p.videoRequests.pop_front();
                        }
                        if (!p.currentAudioRequest && !p.audioRequests.empty())
                        {
                            p.currentAudioRequest = p.audioRequests.front();
                            p.audioRequests.pop_front();
                        }
                    }
                }

                {
                    bool seek = false;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if (p.currentVideoRequest)
                        {
                            if (!time::compareExact(p.currentVideoRequest->time, p.currentVideoTime))
                            {
                                seek = true;
                                p.currentVideoTime = p.currentVideoRequest->time;
                            }
                        }
                    }
                    if (seek)
                    {
                        //std::cout << "video seek: " << p.currentVideoTime << std::endl;

                        if (p.video.avStream != -1)
                        {
                            avcodec_flush_buffers(p.video.avCodecContext[p.video.avStream]);

                            if (av_seek_frame(
                                p.video.avFormatContext,
                                p.video.avStream,
                                av_rescale_q(
                                    p.currentVideoTime.value() - p.info.videoTime.start_time().value(),
                                    swap(p.video.avFormatContext->streams[p.video.avStream]->r_frame_rate),
                                    p.video.avFormatContext->streams[p.video.avStream]->time_base),
                                AVSEEK_FLAG_BACKWARD) < 0)
                            {
                                //! \todo How should this be handled?
                            }
                        }

                        p.video.buffer.clear();
                    }
                }

                if (p.video.avStream != -1 &&
                    p.video.buffer.size() < p.videoBufferSize)
                {
                    AVPacket packet;
                    av_init_packet(&packet);
                    int decoding = 0;
                    bool eof = false;
                    while (0 == decoding)
                    {
                        if (!eof)
                        {
                            decoding = av_read_frame(p.video.avFormatContext, &packet);
                            if (AVERROR_EOF == decoding)
                            {
                                eof = true;
                                decoding = 0;
                            }
                            else if (decoding < 0)
                            {
                                //! \todo How should this be handled?
                                break;
                            }
                        }
                        if ((eof && p.video.avStream != -1) || (p.video.avStream == packet.stream_index))
                        {
                            decoding = avcodec_send_packet(
                                p.video.avCodecContext[p.video.avStream],
                                eof ? nullptr : &packet);
                            if (AVERROR_EOF == decoding)
                            {
                                decoding = 0;
                            }
                            else if (decoding < 0)
                            {
                                //! \todo How should this be handled?
                                break;
                            }
                            decoding = p.decodeVideo();
                            if (AVERROR(EAGAIN) == decoding)
                            {
                                decoding = 0;
                            }
                            else if (AVERROR_EOF == decoding)
                            {
                                break;
                            }
                            else if (decoding < 0)
                            {
                                //! \todo How should this be handled?
                                break;
                            }
                            else if (1 == decoding)
                            {
                                break;
                            }
                        }
                        if (packet.buf)
                        {
                            av_packet_unref(&packet);
                        }
                    }
                    if (packet.buf)
                    {
                        av_packet_unref(&packet);
                    }
                    //std::cout << "video buffer size: " << p.video.buffer.size() << std::endl;
                }

                {
                    std::shared_ptr<Private::VideoRequest> videoRequest;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if ((p.currentVideoRequest && !p.video.buffer.empty()) ||
                            (p.currentVideoRequest && -1 == p.video.avStream))
                        {
                            videoRequest = std::move(p.currentVideoRequest);
                        }
                    }
                    if (videoRequest)
                    {
                        io::VideoData data;
                        data.time = videoRequest->time;
                        if (!p.video.buffer.empty())
                        {
                            data.image = p.video.buffer.front();
                            p.video.buffer.pop_front();
                        }
                        videoRequest->promise.set_value(data);

                        p.currentVideoTime += otime::RationalTime(1.0, p.info.videoTime.duration().rate());
                    }
                }

                {
                    bool seek = false;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if (p.currentAudioRequest)
                        {
                            if (!time::compareExact(p.currentAudioRequest->time.start_time(), p.currentAudioTime))
                            {
                                seek = true;
                                p.currentAudioTime = p.currentAudioRequest->time.start_time();
                            }
                        }
                    }
                    if (seek)
                    {
                        //std::cout << "audio seek: " << p.currentAudioTime << std::endl;

                        if (p.audio.avStream != -1)
                        {
                            avcodec_flush_buffers(p.audio.avCodecContext[p.audio.avStream]);

                            AVRational r;
                            r.num = 1;
                            r.den = p.info.audio.sampleRate;
                            if (av_seek_frame(
                                p.audio.avFormatContext,
                                p.audio.avStream,
                                av_rescale_q(
                                    p.currentAudioTime.value() - p.info.audioTime.start_time().value(),
                                    r,
                                    p.audio.avFormatContext->streams[p.audio.avStream]->time_base),
                                AVSEEK_FLAG_BACKWARD) < 0)
                            {
                                //! \todo How should this be handled?
                            }

                            std::vector<uint8_t> swrOutputBuffer;
                            swrOutputBuffer.resize(
                                static_cast<size_t>(p.info.audio.channelCount)*
                                audio::getByteCount(p.info.audio.dataType)*
                                p.audio.avFrame->nb_samples);
                            uint8_t* swrOutputBufferP[] = { swrOutputBuffer.data() };
                            while (swr_convert(
                                p.audio.swrContext,
                                swrOutputBufferP,
                                p.audio.avFrame->nb_samples,
                                NULL,
                                0) > 0)
                                ;
                            swr_init(p.audio.swrContext);
                        }

                        p.audio.buffer.clear();
                    }
                }

                if (p.audio.avStream != -1 &&
                    audio::getSampleCount(p.audio.buffer) <
                        p.audioBufferSize.rescaled_to(p.info.audio.sampleRate).value())
                {
                    AVPacket packet;
                    av_init_packet(&packet);
                    int decoding = 0;
                    bool eof = false;
                    while (0 == decoding)
                    {
                        if (!eof)
                        {
                            decoding = av_read_frame(p.audio.avFormatContext, &packet);
                            if (AVERROR_EOF == decoding)
                            {
                                eof = true;
                                decoding = 0;
                            }
                            else if (decoding < 0)
                            {
                                //! \todo How should this be handled?
                                break;
                            }
                        }
                        if ((eof && p.audio.avStream != -1) || (p.audio.avStream == packet.stream_index))
                        {
                            decoding = avcodec_send_packet(
                                p.audio.avCodecContext[p.audio.avStream],
                                eof ? nullptr : &packet);
                            if (AVERROR_EOF == decoding)
                            {
                                decoding = 0;
                            }
                            else if (decoding < 0)
                            {
                                //! \todo How should this be handled?
                                break;
                            }
                            decoding = p.decodeAudio();
                            if (AVERROR(EAGAIN) == decoding)
                            {
                                decoding = 0;
                            }
                            else if (AVERROR_EOF == decoding)
                            {
                                const size_t bufferSize = audio::getSampleCount(p.audio.buffer);
                                const size_t bufferSize2 = p.audioBufferSize.rescaled_to(p.info.audio.sampleRate).value();
                                if (bufferSize < bufferSize2)
                                {
                                    auto audio = audio::Audio::create(
                                        p.info.audio,
                                        bufferSize2 - bufferSize);
                                    audio->zero();
                                    p.audio.buffer.push_back(audio);
                                }
                                break;
                            }
                            else if (decoding < 0)
                            {
                                //! \todo How should this be handled?
                                break;
                            }
                            else if (1 == decoding)
                            {
                                break;
                            }
                        }
                        if (packet.buf)
                        {
                            av_packet_unref(&packet);
                        }
                    }
                    if (packet.buf)
                    {
                        av_packet_unref(&packet);
                    }
                    //std::cout << "audio buffer size: " << audio::getSampleCount(p.audio.buffer) << std::endl;
                }

                {
                    const size_t bufferSize = audio::getSampleCount(p.audio.buffer);
                    std::shared_ptr<Private::AudioRequest> audioRequest;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if ((p.currentAudioRequest &&
                                p.currentAudioRequest->time.duration().rescaled_to(p.info.audio.sampleRate).value() <= bufferSize) ||
                            (p.currentAudioRequest && -1 == p.audio.avStream))
                        {
                            audioRequest = std::move(p.currentAudioRequest);
                        }
                    }
                    if (audioRequest)
                    {
                        io::AudioData data;
                        data.time = audioRequest->time.start_time();
                        data.audio = audio::Audio::create(p.info.audio, audioRequest->time.duration().value());
                        data.audio->zero();
                        size_t offset = 0;
                        if (data.time < p.info.audioTime.start_time())
                        {
                            offset = (p.info.audioTime.start_time() - data.time).value() * p.info.audio.getByteCount();
                        }
                        audio::copy(p.audio.buffer, data.audio->getData() + offset, data.audio->getByteCount() - offset);
                        audioRequest->promise.set_value(data);

                        p.currentAudioTime += audioRequest->time.duration();
                    }
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.logTimer = now;
                        const std::string id = string::Format("tl::io::ffmpeg::Read {0}").arg(this);
                        size_t videoRequestsSize = 0;
                        size_t audioRequestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            videoRequestsSize = p.videoRequests.size();
                            audioRequestsSize = p.audioRequests.size();
                        }
                        logSystem->print(id, string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Video requests: {1}\n"
                            "    Audio requests: {2}\n"
                            "    Thread count: {3}").
                            arg(_path.get()).
                            arg(videoRequestsSize).
                            arg(audioRequestsSize).
                            arg(p.threadCount));
                    }
                }
            }
        }

        void Read::_close()
        {
            TLRENDER_P();

            if (p.video.swsContext)
            {
                sws_freeContext(p.video.swsContext);
            }
            if (p.video.avFrame2)
            {
                av_frame_free(&p.video.avFrame2);
            }
            if (p.video.avFrame)
            {
                av_frame_free(&p.video.avFrame);
            }
            for (auto i : p.video.avCodecContext)
            {
                avcodec_close(i.second);
                avcodec_free_context(&i.second);
            }
            for (auto i : p.video.avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (p.video.avIOContext)
            {
                avio_context_free(&p.video.avIOContext);
            }
            //! \bug Free'd by avio_context_free()?
            //if (p.video.avIOContextBuffer)
            //{
            //    av_free(p.video.avIOContextBuffer);
            //}
            if (p.video.avFormatContext)
            {
                avformat_close_input(&p.video.avFormatContext);
            }

            if (p.audio.swrContext)
            {
                swr_free(&p.audio.swrContext);
            }
            if (p.audio.avFrame)
            {
                av_frame_free(&p.audio.avFrame);
            }
            for (auto i : p.audio.avCodecContext)
            {
                avcodec_close(i.second);
                avcodec_free_context(&i.second);
            }
            for (auto i : p.audio.avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (p.audio.avIOContext)
            {
                avio_context_free(&p.audio.avIOContext);
            }
            //! \bug Free'd by avio_context_free()?
            //if (p.audio.avIOContextBuffer)
            //{
            //    av_free(p.audio.avIOContextBuffer);
            //}
            if (p.audio.avFormatContext)
            {
                avformat_close_input(&p.audio.avFormatContext);
            }
        }
    }
}
