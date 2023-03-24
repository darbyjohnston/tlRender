// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
            const Options& options,
            const std::shared_ptr<ReadCache>& readCache)
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
            const auto i = otioTimeline->metadata().find("tl::timeline");
            if (i != otioTimeline->metadata().end())
            {
                try
                {
                    const auto dict = otio::any_cast<otio::AnyDictionary>(i->second);
                    auto j = dict.find("path");
                    if (j != dict.end())
                    {
                        p.path = otio::any_cast<file::Path>(j->second);
                    }
                    j = dict.find("audioPath");
                    if (j != dict.end())
                    {
                        p.audioPath = otio::any_cast<file::Path>(j->second);
                    }
                }
                catch (const std::exception&)
                {}
            }
            p.readCache = readCache ? readCache : ReadCache::create();
            p.readCache->setMax(16);

            // Get information about the timeline.
            otio::ErrorStatus errorStatus;
            auto duration = timeline::getDuration(p.otioTimeline.value, otio::Track::Kind::video);
            if (!duration.has_value())
            {
                duration = timeline::getDuration(p.otioTimeline.value, otio::Track::Kind::audio);
            }
            if (duration.has_value())
            {
                const otime::RationalTime startTime = p.otioTimeline.value->global_start_time().has_value() ?
                    p.otioTimeline.value->global_start_time().value().rescaled_to(duration->rate()) :
                    otime::RationalTime(0, duration->rate());
                p.timeRange = otime::TimeRange(startTime, duration.value());
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
                            p.options.ioOptions["ffmpeg/AudioChannelCount"] = string::Format("{0}").arg(p.ioInfo.audio.channelCount);
                            p.options.ioOptions["ffmpeg/AudioDataType"] = string::Format("{0}").arg(p.ioInfo.audio.dataType);
                            p.options.ioOptions["ffmpeg/AudioSampleRate"] = string::Format("{0}").arg(p.ioInfo.audio.sampleRate);
                            break;
                        }
                    }
                }
            }

            logSystem->print(
                string::Format("tl::timeline::Timeline {0}").arg(this),
                string::Format(
                    "\n"
                    "    Time range: {0}\n"
                    "    Video: {1} {2}\n"
                    "    Audio: {3} {4} {5}").
                arg(p.timeRange).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].size : imaging::Size()).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].pixelType : imaging::PixelType::None).
                arg(p.ioInfo.audio.channelCount).
                arg(p.ioInfo.audio.dataType).
                arg(p.ioInfo.audio.sampleRate));

            // Create a new thread.
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    p.thread.logTimer = std::chrono::steady_clock::now();
                    while (p.thread.running)
                    {
                        p.tick();
                    }
                    p.finishRequests();
                });
        }

        Timeline::Timeline() :
            _p(new Private)
        {}

        Timeline::~Timeline()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
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

        const otime::TimeRange& Timeline::getTimeRange() const
        {
            return _p->timeRange;
        }

        const io::Info& Timeline::getIOInfo() const
        {
            return _p->ioInfo;
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
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
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
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
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
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                videoRequests = std::move(p.mutex.videoRequests);
                audioRequests = std::move(p.mutex.audioRequests);
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(VideoData());
            }
            for (auto& request : audioRequests)
            {
                request->promise.set_value(AudioData());
            }
            p.readCache->cancelRequests();
        }
    }
}
