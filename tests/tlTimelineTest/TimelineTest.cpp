// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/TimelineTest.h>

#include <tlTimeline/Timeline.h>

#include <tlIO/IOSystem.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/imageSequenceReference.h>

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
            imaging::Info imageInfo(16, 16, imaging::PixelType::RGB_U8);
            imageInfo.layout.endian = memory::Endian::MSB;
            const auto image = imaging::Image::create(imageInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(imageInfo);
            ioInfo.videoTime = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
            {
                auto write = _context->getSystem<io::System>()->write(file::Path("Timeline Create.0.ppm"), ioInfo);
                for (size_t i = 0; i < static_cast<size_t>(ioInfo.videoTime.duration().value()); ++i)
                {
                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                }
                auto timeline = Timeline::create("Timeline Create.0.ppm", _context);
                const auto& timelineIOInfo = timeline->getIOInfo();
                TLRENDER_ASSERT(!timelineIOInfo.video.empty());
                TLRENDER_ASSERT(timelineIOInfo.video[0] == imageInfo);
            }
            {
                file::mkdir("Timeline Create");
                auto write = _context->getSystem<io::System>()->write(file::Path("Timeline Create/Timeline Create.0.ppm"), ioInfo);
                for (size_t i = 0; i < static_cast<size_t>(ioInfo.videoTime.duration().value()); ++i)
                {
                    write->writeVideo(otime::RationalTime(i, 24.0), image);
                }
                auto timeline = Timeline::create("Timeline Create/Timeline Create.0.ppm", _context);
                const auto& timelineIOInfo = timeline->getIOInfo();
                TLRENDER_ASSERT(!timelineIOInfo.video.empty());
                TLRENDER_ASSERT(timelineIOInfo.video[0] == imageInfo);
            }
        }

        void TimelineTest::_timeline()
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

            // Write an OTIO timeline.
            auto otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("file://", "Timeline Test.", ".ppm", 0, 1, 1, 0));
            const otime::TimeRange clipTimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
            otioClip->set_source_range(clipTimeRange);
            otio::ErrorStatus errorStatus;
            auto otioTrack = new otio::Track();
            otioTrack->append_child(otioClip, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot append child");
            }
            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "Timeline Test.", ".ppm", 0, 1, 1, 0));
            otioClip->set_source_range(clipTimeRange);
            otioTrack->append_child(otioClip, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot append child");
            }
            auto otioStack = new otio::Stack;
            otioStack->append_child(otioTrack, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot append child");
            }
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
            otioTimeline->set_tracks(otioStack);
            const std::string fileName("Timeline Test.otio");
            otioTimeline->to_json_file(fileName, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot write file: " + fileName);
            }

            // Write the image sequence files.
            imaging::Info imageInfo(16, 16, imaging::PixelType::RGB_U8);
            imageInfo.layout.endian = memory::Endian::MSB;
            const auto image = imaging::Image::create(imageInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(imageInfo);
            ioInfo.videoTime = clipTimeRange;
            auto write = _context->getSystem<io::System>()->write(file::Path("Timeline Test.0.ppm"), ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipTimeRange.duration().value()); ++i)
            {
                write->writeVideo(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline from the OTIO timeline.
            auto timeline = Timeline::create(fileName, _context);
            TLRENDER_ASSERT(timeline->getTimeline());
            TLRENDER_ASSERT(fileName == timeline->getPath().get());
            TLRENDER_ASSERT(Options() == timeline->getOptions());
            const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(48.0, 24.0));
            TLRENDER_ASSERT(time::compareExact(timeRange, timeline->getTimeRange()));
            TLRENDER_ASSERT(imageInfo.size == timeline->getIOInfo().video[0].size);
            TLRENDER_ASSERT(imageInfo.pixelType == timeline->getIOInfo().video[0].pixelType);

            // Get video from the timeline.
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
