// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/TimelineTest.h>

#include <tlTimeline/Timeline.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <feather-tk/core/Format.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        TimelineTest::TimelineTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::TimelineTest")
        {}

        std::shared_ptr<TimelineTest> TimelineTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<TimelineTest>(new TimelineTest(context));
        }

        void TimelineTest::run()
        {
            _enums();
            _options();
            _util();
            _transitions();
            _videoData();
            _timeline();
            _separateAudio();
        }

        void TimelineTest::_enums()
        {
            _enum<ImageSequenceAudio>("ImageSequenceAudio", getImageSequenceAudioEnums);
            _enum<Transition>("Transition", getTransitionEnums);
        }

        void TimelineTest::_options()
        {
            Options a;
            a.imageSequenceAudio = ImageSequenceAudio::FileName;
            FTK_ASSERT(a == a);
            FTK_ASSERT(a != Options());
        }

        void TimelineTest::_util()
        {
        }

        void TimelineTest::_transitions()
        {
            {
                FTK_ASSERT(toTransition(std::string()) == Transition::None);
                FTK_ASSERT(toTransition("SMPTE_Dissolve") == Transition::Dissolve);
            }
        }

        void TimelineTest::_videoData()
        {
            {
                VideoLayer a, b;
                FTK_ASSERT(a == b);
                a.transition = Transition::Dissolve;
                FTK_ASSERT(a != b);
            }
            {
                VideoData a, b;
                FTK_ASSERT(a == b);
                a.time = OTIO_NS::RationalTime(1.0, 24.0);
                FTK_ASSERT(a != b);
            }
        }

        void TimelineTest::_timeline()
        {
            // Test timelines.
            const std::vector<file::Path> paths =
            {
                file::Path(TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v"),
                file::Path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg"),
                file::Path(TLRENDER_SAMPLE_DATA, "MovieAndSeq.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "TransitionGap.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClip.otioz"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClipSeq.otioz")
            };
            for (const auto& path : paths)
            {
                try
                {
                    _print(ftk::Format("Timeline: {0}").arg(path.get()));
                    auto timeline = Timeline::create(_context, path);
                    _timeline(timeline);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
            for (const auto& path : paths)
            {
                try
                {
                    _print(ftk::Format("Memory timeline: {0}").arg(path.get()));
                    auto otioTimeline = timeline::create(_context, path);
                    toMemoryReferences(otioTimeline, path.getDirectory(), ToMemoryReference::Shared);
                    auto timeline = timeline::Timeline::create(_context, otioTimeline);
                    _timeline(timeline);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }

        void TimelineTest::_timeline(const std::shared_ptr<timeline::Timeline>& timeline)
        {
            // Get video from the timeline.
            const OTIO_NS::TimeRange& timeRange = timeline->getTimeRange();
            std::vector<timeline::VideoData> videoData;
            std::vector<timeline::VideoRequest> videoRequests;
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0)));
            }
            io::Options ioOptions;
            ioOptions["Layer"] = "1";
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0), ioOptions));
            }
            while (videoData.size() < static_cast<size_t>(timeRange.duration().value()) * 2)
            {
                auto i = videoRequests.begin();
                while (i != videoRequests.end())
                {
                    if (i->future.valid() &&
                        i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        videoData.push_back(i->future.get());
                        i = videoRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            FTK_ASSERT(videoRequests.empty());

            // Get audio from the timeline.
            std::vector<timeline::AudioData> audioData;
            std::vector<timeline::AudioRequest> audioRequests;
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().rescaled_to(1.0).value()); ++i)
            {
                audioRequests.push_back(timeline->getAudio(i));
            }
            while (audioData.size() < static_cast<size_t>(timeRange.duration().rescaled_to(1.0).value()))
            {
                auto i = audioRequests.begin();
                while (i != audioRequests.end())
                {
                    if (i->future.valid() &&
                        i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        audioData.push_back(i->future.get());
                        i = audioRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            FTK_ASSERT(audioRequests.empty());

            // Cancel requests.
            videoData.clear();
            videoRequests.clear();
            audioData.clear();
            audioRequests.clear();
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0), ioOptions));
            }
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().rescaled_to(1.0).value()); ++i)
            {
                audioRequests.push_back(timeline->getAudio(i));
            }
            std::vector<uint64_t> ids;
            for (const auto& i : videoRequests)
            {
                ids.push_back(i.id);
            }
            for (const auto& i : audioRequests)
            {
                ids.push_back(i.id);
            }
            timeline->cancelRequests(ids);
        }

        void TimelineTest::_separateAudio()
        {
#if defined(TLRENDER_FFMPEG)
            try
            {
                const file::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                const file::Path audioPath(TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v");
                auto timeline = Timeline::create(_context, path.get(), audioPath.get());
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                const file::Path audioPath(TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v");
                auto timeline = Timeline::create(_context, path, audioPath);
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                Options options;
                options.imageSequenceAudio = ImageSequenceAudio::None;
                auto timeline = Timeline::create(_context, path, options);
                const file::Path& audioPath = timeline->getAudioPath();
                FTK_ASSERT(audioPath.isEmpty());
                _print(ftk::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                Options options;
                options.imageSequenceAudio = ImageSequenceAudio::Extension;
                auto timeline = Timeline::create(_context, path, options);
                const file::Path& audioPath = timeline->getAudioPath();
                FTK_ASSERT(!audioPath.isEmpty());
                _print(ftk::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                Options options;
                options.imageSequenceAudio = ImageSequenceAudio::FileName;
                options.imageSequenceAudioFileName = file::Path(
                    TLRENDER_SAMPLE_DATA, "AudioToneStereo.wav").get();
                auto timeline = Timeline::create(_context, path, options);
                const file::Path& audioPath = timeline->getAudioPath();
                FTK_ASSERT(!audioPath.isEmpty());
                _print(ftk::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
#endif // TLRENDER_FFMPEG
        }
    }
}
