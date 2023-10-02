// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/TimelineTest.h>

#include <tlTimeline/Timeline.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        TimelineTest::TimelineTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::TimelineTest", context)
        {}

        std::shared_ptr<TimelineTest> TimelineTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<TimelineTest>(new TimelineTest(context));
        }

        void TimelineTest::run()
        {
            _enums();
            _util();
            _transitions();
            _videoData();
            _create();
            _timeline();
            _imageSequence();
        }

        void TimelineTest::_enums()
        {
            _enum<Transition>("Transition", getTransitionEnums);
        }

        void TimelineTest::_util()
        {
            for (const auto& i : getExtensions(
                static_cast<int>(io::FileType::Movie) |
                static_cast<int>(io::FileType::Sequence) |
                static_cast<int>(io::FileType::Audio),
                _context))
            {
                std::stringstream ss;
                ss << "Timeline extension: " << i;
                _print(ss.str());
            }
            for (const auto& path : getPaths(
                file::Path(TLRENDER_SAMPLE_DATA),
                file::PathOptions(),
                _context))
            {
                _print(string::Format("Path: {0}").arg(path.get()));
            }
        }

        void TimelineTest::_transitions()
        {
            {
                TLRENDER_ASSERT(toTransition(std::string()) == Transition::None);
                TLRENDER_ASSERT(toTransition("SMPTE_Dissolve") == Transition::Dissolve);
            }
        }

        void TimelineTest::_videoData()
        {
            {
                VideoLayer a, b;
                TLRENDER_ASSERT(a == b);
                a.transition = Transition::Dissolve;
                TLRENDER_ASSERT(a != b);
            }
            {
                VideoData a, b;
                TLRENDER_ASSERT(a == b);
                a.time = otime::RationalTime(1.0, 24.0);
                TLRENDER_ASSERT(a != b);
            }
        }

        void TimelineTest::_create()
        {
            image::Info imageInfo(16, 16, image::PixelType::RGB_U8);
            imageInfo.layout.endian = memory::Endian::MSB;
            const auto image = image::Image::create(imageInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(imageInfo);
            ioInfo.videoTime = otime::TimeRange(
                otime::RationalTime(0.0, 24.0),
                otime::RationalTime(24.0, 24.0));
            {
                auto write = _context->getSystem<io::System>()->write(
                    file::Path("Timeline Create.0.ppm"),
                    ioInfo);
                for (size_t i = 0;
                    i < static_cast<size_t>(ioInfo.videoTime.duration().value());
                    ++i)
                {
                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                }
                auto timeline = Timeline::create(
                    "Timeline Create.0.ppm",
                    _context);
                const auto& timelineIOInfo = timeline->getIOInfo();
                TLRENDER_ASSERT(!timelineIOInfo.video.empty());
                TLRENDER_ASSERT(timelineIOInfo.video[0] == imageInfo);
            }
            {
                file::mkdir("Timeline Create");
                auto write = _context->getSystem<io::System>()->write(
                    file::Path("Timeline Create/Timeline Create.0.ppm"),
                    ioInfo);
                for (size_t i = 0;
                    i < static_cast<size_t>(ioInfo.videoTime.duration().value());
                    ++i)
                {
                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                }
                auto timeline = Timeline::create(
                    "Timeline Create/Timeline Create.0.ppm",
                    _context);
                const auto& timelineIOInfo = timeline->getIOInfo();
                TLRENDER_ASSERT(!timelineIOInfo.video.empty());
                TLRENDER_ASSERT(timelineIOInfo.video[0] == imageInfo);
            }
        }

        void TimelineTest::_timeline()
        {
            // Write an OTIO timeline.
            /*auto otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference(
                "file://", "Timeline Test.", ".ppm", 0, 1, 1, 0));
            otioClip->set_source_range(otime::TimeRange(
                otime::RationalTime(0.0, 24.0),
                otime::RationalTime(24.0, 24.0)));
            auto otioTrack = new otio::Track();
            otioTrack->append_child(otioClip);

            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference(
                "", "Timeline Test.", ".ppm", 0, 1, 1, 0));
            otioClip->set_source_range(otime::TimeRange(
                otime::RationalTime(0.0, 24.0),
                otime::RationalTime(24.0, 24.0)));
            otioTrack->append_child(otioClip);

#if defined(TLRENDER_FFMPEG)
            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ExternalReference(
                "Timeline Test.mov"));
            otioClip->set_source_range(otime::TimeRange(
                otime::RationalTime(0.0, 24.0),
                otime::RationalTime(24.0, 24.0)));
            otioTrack->append_child(otioClip);
#endif // TLRENDER_FFMPEG

            otime::TimeRange timeRange = otioTrack->available_range();
            auto otioStack = new otio::Stack;
            otioStack->append_child(otioTrack);
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(
                new otio::Timeline);
            otioTimeline->set_tracks(otioStack);
            const std::string fileName("Timeline Test.otio");
            otioTimeline->to_json_file(fileName);

            // Write the media files.
            {
                image::Info imageInfo(16, 16, image::PixelType::RGB_U8);
                imageInfo.layout.endian = memory::Endian::MSB;
                const auto image = image::Image::create(imageInfo);
                io::Info ioInfo;
                ioInfo.video.push_back(imageInfo);
                ioInfo.videoTime = otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(24.0, 24.0));
                auto write = _context->getSystem<io::System>()->write(file::Path("Timeline Test.0.ppm"), ioInfo);
                for (size_t i = 0; i < static_cast<size_t>(ioInfo.videoTime.duration().value()); ++i)
                {
                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                }
            }
#if defined(TLRENDER_FFMPEG)
            {
                image::Info imageInfo(16, 16, image::PixelType::RGB_U8);
                const auto image = image::Image::create(imageInfo);
                io::Info ioInfo;
                ioInfo.video.push_back(imageInfo);
                ioInfo.videoTime = otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(24.0, 24.0));
                auto write = _context->getSystem<io::System>()->write(file::Path("Timeline Test.mov"), ioInfo);
                for (size_t i = 0; i < static_cast<size_t>(ioInfo.videoTime.duration().value()); ++i)
                {
                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                }
            }
#endif // TLRENDER_FFMPEG*/

            // Test timelines.
            const std::vector<file::Path> paths =
            {
                //file::Path(TLRENDER_SAMPLE_DATA, "AudioTones.otio"),
                //file::Path(TLRENDER_SAMPLE_DATA, "AudioTonesAndVideo.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "Gap.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "MovieAndSeq.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "TransitionOverlay.otio")
            };
            for (const auto& path : paths)
            {
                _print(string::Format("Timeline: {0}").arg(path.get()));
                auto timeline = Timeline::create(path, _context);
                TLRENDER_ASSERT(timeline->getTimeline());
                TLRENDER_ASSERT(path == timeline->getPath());
                _timeline(timeline);
            }
            for (const auto& path : paths)
            {
                _print(string::Format("Memory timeline: {0}").arg(path.get()));
                auto otioTimeline = timeline::create(path, _context);
                toMemoryReferences(otioTimeline, path.getDirectory());
                auto timeline = timeline::Timeline::create(otioTimeline, _context);
                TLRENDER_ASSERT(timeline->getTimeline());
                TLRENDER_ASSERT(path == timeline->getPath());
                _timeline(timeline);
            }
        }

        void TimelineTest::_timeline(const std::shared_ptr<timeline::Timeline>& timeline)
        {
            // Get video from the timeline.
            const otime::TimeRange& timeRange = timeline->getTimeRange();
            std::vector<timeline::VideoData> videoData;
            std::vector<std::future<timeline::VideoData> > futures;
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0), 1));
            }
            while (videoData.size() < static_cast<size_t>(timeRange.duration().value()) * 2)
            {
                auto i = futures.begin();
                while (i != futures.end())
                {
                    if (i->valid() &&
                        i->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        videoData.push_back(i->get());
                        i = futures.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            TLRENDER_ASSERT(futures.empty());

            // Cancel requests.
            videoData.clear();
            futures.clear();
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0), 1));
            }
            timeline->cancelRequests();
        }

        void TimelineTest::_imageSequence()
        {
            //! \bug This uses the image sequence created by _timeline().
            auto timeline = Timeline::create("Timeline Test.0.ppm", _context);
            {
                std::stringstream ss;
                ss << timeline->getTimeRange().duration();
                _print(ss.str());
            }
            TLRENDER_ASSERT(
                otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0)) ==
                timeline->getTimeRange());
        }
    }
}
