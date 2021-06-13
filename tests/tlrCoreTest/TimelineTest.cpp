// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TimelineTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/IO.h>
#include <tlrCore/Timeline.h>

#include <opentimelineio/timeline.h>
#include <opentimelineio/imageSequenceReference.h>

using namespace tlr::timeline;

namespace tlr
{
    namespace CoreTest
    {
        TimelineTest::TimelineTest() :
            ITest("CoreTest::TimelineTest")
        {}

        std::shared_ptr<TimelineTest> TimelineTest::create()
        {
            return std::shared_ptr<TimelineTest>(new TimelineTest);
        }

        void TimelineTest::run()
        {
            _toRanges();
            _timeline();
        }

        void TimelineTest::_toRanges()
        {
            {
                std::vector<otime::RationalTime> f;
                auto r = toRanges(f);
                TLR_ASSERT(r.empty());
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24)
                };
                auto r = toRanges(f);
                TLR_ASSERT(1 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(1, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24)
                };
                auto r = toRanges(f);
                TLR_ASSERT(1 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(2, 24)
                };
                auto r = toRanges(f);
                TLR_ASSERT(1 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(3, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(2, 24)
                };
                auto r = toRanges(f);
                TLR_ASSERT(2 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(1, 24)) == r[0]);
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(2, 24), otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24)
                };
                auto r = toRanges(f);
                TLR_ASSERT(2 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(3, 24), otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24),
                    otime::RationalTime(4, 24)
                };
                auto r = toRanges(f);
                TLR_ASSERT(2 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(3, 24), otime::RationalTime(2, 24)) == r[1]);
            }
        }

        void TimelineTest::_timeline()
        {
            for (const auto& i : getExtensions())
            {
                std::stringstream ss;
                ss << "Timeline extension: " << i;
                _print(ss.str());
            }
            
            // Write an OTIO timeline.
            auto otioTrack = new otio::Track();
            auto otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelineTest.", ".png", 0, 1, 1, 0));
            const otime::RationalTime clipDuration(24.0, 24.0);
            otioClip->set_source_range(otime::TimeRange(otime::RationalTime(0.0, 24.0), clipDuration));
            otio::ErrorStatus errorStatus = otio::ErrorStatus::OK;
            otioTrack->append_child(otioClip, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot append child");
            }
            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelineTest.", ".png", 0, 1, 1, 0));
            otioClip->set_source_range(otime::TimeRange(otime::RationalTime(0.0, 24.0), clipDuration));
            otioTrack->append_child(otioClip, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot append child");
            }
            auto otioStack = new otio::Stack;
            otioStack->append_child(otioTrack, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot append child");
            }
            auto otioTimeline = new otio::Timeline;
            otioTimeline->set_tracks(otioStack);
            std::string fileName = "TimelineTest.otio";
            otioTimeline->to_json_file(fileName, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot write file: " + fileName);
            }

            // Write the image sequence files.
            const imaging::Info imageInfo(16, 16, imaging::PixelType::RGB_U8);
            const auto image = imaging::Image::create(imageInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(io::VideoInfo(imageInfo, clipDuration));
            auto ioSystem = io::System::create();
            auto write = ioSystem->write("TimelineTest.0.png", ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipDuration.value()); ++i)
            {
                write->writeVideoFrame(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline from the OTIO timeline.
            auto timeline = Timeline::create(fileName);
            TLR_ASSERT(fileName == timeline->getFileName());
            const otime::RationalTime timelineDuration(48.0, 24.0);
            TLR_ASSERT(timelineDuration == timeline->getDuration());
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == timeline->getGlobalStartTime());
            TLR_ASSERT(imageInfo == timeline->getImageInfo());
            TLR_ASSERT(std::vector<otime::TimeRange>(
                {
                    otime::TimeRange(otime::RationalTime(0.0, 24.0), clipDuration),
                    otime::TimeRange(otime::RationalTime(24.0, 24.0), clipDuration)
                }) == timeline->getClipRanges());

            // Render frames from the timeline.
            std::vector<io::VideoFrame> frames;
            std::vector<std::future<io::VideoFrame> > futures;
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->render(otime::RationalTime(i, 24.0)));
            }
            while (frames.size() < static_cast<size_t>(timelineDuration.value()))
            {
                timeline->tick();
                auto i = futures.begin();
                while (i != futures.end())
                {
                    if (i->valid() &&
                        i->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        frames.push_back(i->get());
                        i = futures.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }

            // Render frames from the timeline, setting the active range.
            timeline->setActiveRanges({ otime::TimeRange(otime::RationalTime(0.0, 24.0), timelineDuration) });
            frames.clear();
            futures.clear();
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->render(otime::RationalTime(i, 24.0)));
            }
            while (frames.size() < static_cast<size_t>(timelineDuration.value()))
            {
                timeline->tick();
                auto i = futures.begin();
                while (i != futures.end())
                {
                    if (i->valid() &&
                        i->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        frames.push_back(i->get());
                        i = futures.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }

            // Cancel renders.
            frames.clear();
            futures.clear();
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->render(otime::RationalTime(i, 24.0)));
            }
            timeline->cancelRenders();
        }
    }
}
