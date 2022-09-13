// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        std::vector<std::string> getExtensions(
            int types,
            const std::shared_ptr<system::Context>& context)
        {
            std::vector<std::string> out;
            //! \todo Get extensions for the Python adapters.
            if (types & static_cast<int>(io::FileType::Movie))
            {
                out.push_back(".otio");
            }
            if (auto ioSystem = context->getSystem<io::System>())
            {
                for (const auto& plugin : ioSystem->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions(types);
                    out.insert(out.end(), extensions.begin(), extensions.end());
                }
            }
            return out;
        }

        TLRENDER_ENUM_IMPL(
            FileSequenceAudio,
            "None",
            "BaseName",
            "FileName",
            "Directory");
        TLRENDER_ENUM_SERIALIZE_IMPL(FileSequenceAudio);

        bool Options::operator == (const Options& other) const
        {
            return fileSequenceAudio == other.fileSequenceAudio &&
                fileSequenceAudioFileName == other.fileSequenceAudioFileName &&
                fileSequenceAudioDirectory == other.fileSequenceAudioDirectory &&
                videoRequestCount == other.videoRequestCount &&
                audioRequestCount == other.audioRequestCount &&
                requestTimeout == other.requestTimeout &&
                ioOptions == other.ioOptions &&
                pathOptions == other.pathOptions;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        void Timeline::_init(
            const otio::SerializableObject::Retainer<otio::Timeline>& otioTimeline,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(string::Format("    File sequence audio: {0}").
                    arg(options.fileSequenceAudio));
                lines.push_back(string::Format("    File sequence audio file name: {0}").
                    arg(options.fileSequenceAudioFileName));
                lines.push_back(string::Format("    File sequence audio directory: {0}").
                    arg(options.fileSequenceAudioDirectory));
                lines.push_back(string::Format("    Video request count: {0}").
                    arg(options.videoRequestCount));
                lines.push_back(string::Format("    Audio request count: {0}").
                    arg(options.audioRequestCount));
                lines.push_back(string::Format("    Request timeout: {0}ms").
                    arg(options.requestTimeout.count()));
                for (const auto& i : options.ioOptions)
                {
                    lines.push_back(string::Format("    AV I/O {0}: {1}").
                        arg(i.first).
                        arg(i.second));
                }
                lines.push_back(string::Format("    Path max number digits: {0}").
                    arg(options.pathOptions.maxNumberDigits));
                logSystem->print(
                    string::Format("tl::timeline::Timeline {0}").arg(this),
                    string::join(lines, "\n"));
            }

            p.context = context;
            p.options = options;
            p.otioTimeline = otioTimeline;

            // Get information about the timeline.
            otio::ErrorStatus errorStatus;
            auto duration = timeline::getDuration(p.otioTimeline.value, otio::Track::Kind::video);
            if (!duration.has_value())
            {
                duration = timeline::getDuration(p.otioTimeline.value, otio::Track::Kind::audio);
            }
            if (duration.has_value())
            {
                p.duration = duration.value();
            }
            if (p.otioTimeline.value->global_start_time().has_value())
            {
                p.globalStartTime = p.otioTimeline.value->global_start_time().value();
            }
            else
            {
                p.globalStartTime = otime::RationalTime(0, p.duration.rate());
            }
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const otio::Track*>(i.value))
                {
                    if (otio::Track::Kind::video == otioTrack->kind())
                    {
                        if (p.getVideoInfo(otioTrack))
                        {
                            break;
                        }
                    }
                }
            }
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const otio::Track*>(i.value))
                {
                    if (otio::Track::Kind::audio == otioTrack->kind())
                    {
                        if (p.getAudioInfo(otioTrack))
                        {
                            break;
                        }
                    }
                }
            }

            logSystem->print(
                string::Format("tl::timeline::Timeline {0}").arg(this),
                string::Format(
                    "\n"
                    "    Duration: {0}\n"
                    "    Global start time: {1}\n"
                    "    Video: {2} {3}\n"
                    "    Audio: {4} {5} {6}").
                arg(p.duration).
                arg(p.globalStartTime).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].size : imaging::Size()).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].pixelType : imaging::PixelType::None).
                arg(p.ioInfo.audio.channelCount).
                arg(p.ioInfo.audio.dataType).
                arg(p.ioInfo.audio.sampleRate));

            // Create a new thread.
            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLRENDER_P();

                    p.logTimer = std::chrono::steady_clock::now();

                    while (p.running)
                    {
                        p.tick();
                    }

                    {
                        std::list<std::shared_ptr<Private::VideoRequest> > videoRequestsCleanup;
                        std::list<std::shared_ptr<Private::AudioRequest> > audioRequestsCleanup;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            p.stopped = true;
                            while (!p.videoRequests.empty())
                            {
                                videoRequestsCleanup.push_back(p.videoRequests.front());
                                p.videoRequests.pop_front();
                            }
                            while (!p.audioRequests.empty())
                            {
                                audioRequestsCleanup.push_back(p.audioRequests.front());
                                p.audioRequests.pop_front();
                            }
                        }
                        while (!p.videoRequestsInProgress.empty())
                        {
                            videoRequestsCleanup.push_back(p.videoRequestsInProgress.front());
                            p.videoRequestsInProgress.pop_front();
                        }
                        while (!p.audioRequestsInProgress.empty())
                        {
                            audioRequestsCleanup.push_back(p.audioRequestsInProgress.front());
                            p.audioRequestsInProgress.pop_front();
                        }
                        for (auto& request : videoRequestsCleanup)
                        {
                            VideoData data;
                            data.time = request->time;
                            for (auto& i : request->layerData)
                            {
                                VideoLayer layer;
                                if (i.image.valid())
                                {
                                    layer.image = i.image.get().image;
                                }
                                if (i.imageB.valid())
                                {
                                    layer.imageB = i.imageB.get().image;
                                }
                                layer.transition = i.transition;
                                layer.transitionValue = i.transitionValue;
                                data.layers.push_back(layer);
                            }
                            request->promise.set_value(data);
                        }
                        for (auto& request : audioRequestsCleanup)
                        {
                            AudioData data;
                            data.seconds = request->seconds;
                            for (auto& i : request->layerData)
                            {
                                AudioLayer layer;
                                if (i.audio.valid())
                                {
                                    layer.audio = i.audio.get().audio;
                                }
                                data.layers.push_back(layer);
                            }
                            request->promise.set_value(data);
                        }
                    }
                });
        }

        Timeline::Timeline() :
            _p(new Private)
        {}

        Timeline::~Timeline()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        const std::weak_ptr<system::Context>& Timeline::getContext() const
        {
            return _p->context;
        }
        
        const otio::SerializableObject::Retainer<otio::Timeline>& Timeline::getTimeline() const
        {
            return _p->otioTimeline;
        }

        const file::Path& Timeline::getPath() const
        {
            return _p->path;
        }

        const file::Path& Timeline::getAudioPath() const
        {
            return _p->audioPath;
        }

        const Options& Timeline::getOptions() const
        {
            return _p->options;
        }

        const otime::RationalTime& Timeline::getGlobalStartTime() const
        {
            return _p->globalStartTime;
        }

        const otime::RationalTime& Timeline::getDuration() const
        {
            return _p->duration;
        }

        const io::Info& Timeline::getIOInfo() const
        {
            return _p->ioInfo;
        }

        void Timeline::setActiveRanges(const std::vector<otime::TimeRange>& ranges)
        {
            _p->activeRanges = ranges;
        }

        std::future<VideoData> Timeline::getVideo(const otime::RationalTime& time, uint16_t videoLayer)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->videoLayer = videoLayer;
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
                p.requestCV.notify_one();
            }
            else
            {
                request->promise.set_value(VideoData());
            }
            return future;
        }

        std::future<AudioData> Timeline::getAudio(int64_t seconds)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->seconds = seconds;
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
                p.requestCV.notify_one();
            }
            else
            {
                request->promise.set_value(AudioData());
            }
            return future;
        }

        void Timeline::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequestsCleanup;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequestsCleanup;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                videoRequestsCleanup = std::move(p.videoRequests);
                audioRequestsCleanup = std::move(p.audioRequests);
            }
            for (auto& request : videoRequestsCleanup)
            {
                request->promise.set_value(VideoData());
            }
            for (auto& request : audioRequestsCleanup)
            {
                request->promise.set_value(AudioData());
            }
            for (auto& i : p.readers)
            {
                i.second.read->cancelRequests();
            }
        }
    }
}
