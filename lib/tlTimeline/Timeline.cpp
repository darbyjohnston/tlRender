// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/LogSystem.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            const size_t readCacheMax = 10;
        }

        void Timeline::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& otioTimeline,
            const Options& options)
        {
            FEATHER_TK_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(feather_tk::Format("    Image sequence audio: {0}").
                    arg(options.imageSequenceAudio));
                lines.push_back(feather_tk::Format("    Image sequence audio extensions: {0}").
                    arg(feather_tk::join(options.imageSequenceAudioExtensions, ", ")));
                lines.push_back(feather_tk::Format("    Image sequence audio file name: {0}").
                    arg(options.imageSequenceAudioFileName));
                lines.push_back(feather_tk::Format("    Compatability: {0}").
                    arg(options.compat));
                lines.push_back(feather_tk::Format("    Video request max: {0}").
                    arg(options.videoRequestMax));
                lines.push_back(feather_tk::Format("    Audio request max: {0}").
                    arg(options.audioRequestMax));
                lines.push_back(feather_tk::Format("    Request timeout: {0}ms").
                    arg(options.requestTimeout.count()));
                for (const auto& i : options.ioOptions)
                {
                    lines.push_back(feather_tk::Format("    AV I/O {0}: {1}").
                        arg(i.first).
                        arg(i.second));
                }
                lines.push_back(feather_tk::Format("    Path max number digits: {0}").
                    arg(options.pathOptions.maxNumberDigits));
                logSystem->print(
                    feather_tk::Format("tl::timeline::Timeline {0}").arg(this),
                    feather_tk::join(lines, "\n"));
            }

            p.context = context;
            p.otioTimeline = otioTimeline;
            p.timelineChanges = feather_tk::ObservableValue<bool>::create(false);
            const auto i = otioTimeline->metadata().find("tlRender");
            if (i != otioTimeline->metadata().end())
            {
                try
                {
                    const auto dict = std::any_cast<OTIO_NS::AnyDictionary>(i->second);
                    auto j = dict.find("path");
                    if (j != dict.end())
                    {
                        p.path = file::Path(std::any_cast<std::string>(j->second));
                    }
                    j = dict.find("audioPath");
                    if (j != dict.end())
                    {
                        p.audioPath = file::Path(std::any_cast<std::string>(j->second));
                    }
                }
                catch (const std::exception&)
                {}
            }
            p.options = options;
            p.readCache.setMax(readCacheMax);

            // Get information about the timeline.
            p.timeRange = timeline::getTimeRange(p.otioTimeline.value);
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const OTIO_NS::Track*>(i.value))
                {
                    if (OTIO_NS::Track::Kind::audio == otioTrack->kind())
                    {
                        if (p.getAudioInfo(otioTrack))
                        {
                            auto j = p.options.ioOptions.find("FFmpeg/AudioChannelCount");
                            if (j == p.options.ioOptions.end())
                            {
                                p.options.ioOptions["FFmpeg/AudioChannelCount"] =
                                    feather_tk::Format("{0}").arg(p.ioInfo.audio.channelCount);
                            }
                            j = p.options.ioOptions.find("FFmpeg/AudioDataType");
                            if (j == p.options.ioOptions.end())
                            {
                                p.options.ioOptions["FFmpeg/AudioDataType"] =
                                    feather_tk::Format("{0}").arg(p.ioInfo.audio.dataType);
                            }
                            j = p.options.ioOptions.find("FFmpeg/AudioSampleRate");
                            if (j == p.options.ioOptions.end())
                            {
                                p.options.ioOptions["FFmpeg/AudioSampleRate"] =
                                    feather_tk::Format("{0}").arg(p.ioInfo.audio.sampleRate);
                            }
                            break;
                        }
                    }
                }
            }
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const OTIO_NS::Track*>(i.value))
                {
                    if (OTIO_NS::Track::Kind::video == otioTrack->kind())
                    {
                        if (p.getVideoInfo(otioTrack))
                        {
                            break;
                        }
                    }
                }
            }

            logSystem->print(
                feather_tk::Format("tl::timeline::Timeline {0}").arg(this),
                feather_tk::Format(
                    "\n"
                    "    Time range: {0}\n"
                    "    Video: {1} {2}\n"
                    "    Audio: {3} {4} {5}").
                arg(p.timeRange).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].size : feather_tk::Size2I()).
                arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].type : feather_tk::ImageType::None).
                arg(p.ioInfo.audio.channelCount).
                arg(p.ioInfo.audio.dataType).
                arg(p.ioInfo.audio.sampleRate));

            // Create a new thread.
            p.mutex.otioTimeline = p.otioTimeline;
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    FEATHER_TK_P();
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
            FEATHER_TK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<feather_tk::Context> Timeline::getContext() const
        {
            return _p->context.lock();
        }
        
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& Timeline::getTimeline() const
        {
            return _p->otioTimeline;
        }

        std::shared_ptr<feather_tk::IObservableValue<bool> > Timeline::observeTimelineChanges() const
        {
            return _p->timelineChanges;
        }

        void Timeline::setTimeline(const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& value)
        {
            FEATHER_TK_P();
            p.otioTimeline = value;
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            if (!p.mutex.stopped)
            {
                p.mutex.otioTimeline = value;
            }
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

        const OTIO_NS::TimeRange& Timeline::getTimeRange() const
        {
            return _p->timeRange;
        }

        const io::Info& Timeline::getIOInfo() const
        {
            return _p->ioInfo;
        }

        VideoSizeRequest Timeline::getVideoSize(
            const OTIO_NS::RationalTime&,
            const io::Options&)
        {
            FEATHER_TK_P();
            VideoSizeRequest out;
            std::promise<size_t> promise;
            out.future = promise.get_future();
            size_t byteCount = 0;
            if (!p.ioInfo.video.empty())
            {
                byteCount = p.ioInfo.video.front().getByteCount();
            }
            promise.set_value(byteCount);
            return out;
        }

        VideoRequest Timeline::getVideo(
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            FEATHER_TK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::VideoRequest>();
            request->id = p.requestId;
            request->time = time;
            request->options = options;
            VideoRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
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
            return out;
        }

        AudioSizeRequest Timeline::getAudioSize(
            double,
            const io::Options&)
        {
            FEATHER_TK_P();
            AudioSizeRequest out;
            std::promise<size_t> promise;
            out.future = promise.get_future();
            promise.set_value(p.ioInfo.audio.sampleRate * p.ioInfo.audio.getByteCount());
            return out;
        }

        AudioRequest Timeline::getAudio(
            double seconds,
            const io::Options& options)
        {
            FEATHER_TK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::AudioRequest>();
            request->id = p.requestId;
            request->seconds = seconds;
            request->options = options;
            AudioRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
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
            return out;
        }

        void Timeline::cancelRequests(const std::vector<uint64_t>& ids)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            {
                auto i = p.mutex.videoRequests.begin();
                while (i != p.mutex.videoRequests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.mutex.videoRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                auto i = p.mutex.audioRequests.begin();
                while (i != p.mutex.audioRequests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.mutex.audioRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
        }

        void Timeline::tick()
        {
            FEATHER_TK_P();
            bool otioTimelineChanged = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                otioTimelineChanged = p.mutex.otioTimelineChanged;
                p.mutex.otioTimelineChanged = false;
            }
            if (otioTimelineChanged)
            {
                p.timelineChanges->setAlways(true);
            }
        }
    }
}
