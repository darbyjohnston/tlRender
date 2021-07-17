// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TimelineTest.h>

#include <tlrCore/AVIOSystem.h>
#include <tlrCore/Assert.h>
#include <tlrCore/Timeline.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/imageSequenceReference.h>

using namespace tlr::timeline;

namespace tlr
{
    namespace CoreTest
    {
        TimelineTest::TimelineTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::TimelineTest", context)
        {}

        std::shared_ptr<TimelineTest> TimelineTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<TimelineTest>(new TimelineTest(context));
        }

        void TimelineTest::run()
        {
            _enums();
            _ranges();
            _transitions();
            _frames();
            _timeline();
        }

        void TimelineTest::_enums()
        {
            _enum<Transition>("Transition", getTransitionEnums);
        }
        
        void TimelineTest::_ranges()
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

        void TimelineTest::_transitions()
        {
            {
                TLR_ASSERT(toTransition(std::string()) == Transition::None);
                TLR_ASSERT(toTransition("SMPTE_Dissolve") == Transition::Dissolve);
            }
        }
        
        void TimelineTest::_frames()
        {
            {
                FrameLayer a, b;
                TLR_ASSERT(a == b);
                a.transition = Transition::Dissolve;
                TLR_ASSERT(a != b);
            }
            {
                Frame a, b;
                TLR_ASSERT(a == b);
                a.time = otime::RationalTime(1.0, 24.0);
                TLR_ASSERT(a != b);
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
            const file::Path path("TimelineTest.otio");
            otioTimeline->to_json_file(path.get(), &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot write file: " + path.get());
            }

            // Write the image sequence files.
            const imaging::Info imageInfo(16, 16, imaging::PixelType::RGB_U8);
            const auto image = imaging::Image::create(imageInfo);
            avio::Info ioInfo;
            ioInfo.video.push_back(imageInfo);
            ioInfo.videoDuration = clipDuration;
            auto write = _context->getSystem<avio::System>()->write(file::Path("TimelineTest.0.png"), ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipDuration.value()); ++i)
            {
                write->writeVideoFrame(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline from the OTIO timeline.
            auto timeline = Timeline::create(path, _context);
            TLR_ASSERT(path == timeline->getPath());
            const otime::RationalTime timelineDuration(48.0, 24.0);
            TLR_ASSERT(timelineDuration == timeline->getDuration());
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == timeline->getGlobalStartTime());
            TLR_ASSERT(imageInfo == timeline->getImageInfo());

            // Get frames from the timeline.
            std::vector<timeline::Frame> frames;
            std::vector<std::future<timeline::Frame> > futures;
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getFrame(otime::RationalTime(i, 24.0)));
            }
            while (frames.size() < static_cast<size_t>(timelineDuration.value()))
            {
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

            // Get frames from the timeline, setting the active range.
            timeline->setActiveRanges({ otime::TimeRange(otime::RationalTime(0.0, 24.0), timelineDuration) });
            frames.clear();
            futures.clear();
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getFrame(otime::RationalTime(i, 24.0)));
            }
            while (frames.size() < static_cast<size_t>(timelineDuration.value()))
            {
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

            // Cancel frames.
            frames.clear();
            futures.clear();
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getFrame(otime::RationalTime(i, 24.0)));
            }
            timeline->cancelFrames();
        }
    }
}
