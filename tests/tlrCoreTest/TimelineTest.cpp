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
            _util();
            _transitions();
            _frames();
            _timeline();
            _imageSequence();
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
        
        void TimelineTest::_util()
        {
            {
                auto otioClip = new otio::Clip;
                otio::ErrorStatus errorStatus;
                auto otioTrack = new otio::Track();
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
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                otioTimeline->set_tracks(otioStack);
                TLR_ASSERT(otioStack == getRoot(otioClip));
                TLR_ASSERT(otioStack == getParent<otio::Stack>(otioClip));
                TLR_ASSERT(otioTrack == getParent<otio::Track>(otioClip));
            }
            {
                Frame a;
                a.time = otime::RationalTime(1.0, 24.0);
                Frame b;
                b.time = otime::RationalTime(1.0, 24.0);
                TLR_ASSERT(isTimeEqual(a, b));
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
            auto otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelineTest.", ".ppm", 0, 1, 1, 0));
            const otime::TimeRange clipTimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
            otioClip->set_source_range(clipTimeRange);
            otio::ErrorStatus errorStatus;
            auto otioTrack = new otio::Track();
            otioTrack->append_child(otioClip, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot append child");
            }
            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelineTest.", ".ppm", 0, 1, 1, 0));
            otioClip->set_source_range(clipTimeRange);
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
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
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
            ioInfo.videoTimeRange = clipTimeRange;
            auto write = _context->getSystem<avio::System>()->write(file::Path("TimelineTest.0.ppm"), ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipTimeRange.duration().value()); ++i)
            {
                write->writeVideoFrame(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline from the OTIO timeline.
            auto timeline = Timeline::create(path, _context);
            TLR_ASSERT(timeline->getTimeline());
            TLR_ASSERT(path == timeline->getPath());
            TLR_ASSERT(Options() == timeline->getOptions());
            const otime::RationalTime timelineDuration(48.0, 24.0);
            TLR_ASSERT(timelineDuration == timeline->getDuration());
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == timeline->getGlobalStartTime());
            TLR_ASSERT(imageInfo.size == timeline->getVideoInfo()[0].size);
            TLR_ASSERT(imageInfo.pixelType == timeline->getVideoInfo()[0].pixelType);

            // Get frames from the timeline.
            std::vector<timeline::Frame> frames;
            std::vector<std::future<timeline::Frame> > futures;
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getFrame(otime::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getFrame(otime::RationalTime(i, 24.0), 1));
            }
            while (frames.size() < static_cast<size_t>(timelineDuration.value()) * 2)
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
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getFrame(otime::RationalTime(i, 24.0), 1));
            }
            while (frames.size() < static_cast<size_t>(timelineDuration.value()) * 2)
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
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getFrame(otime::RationalTime(i, 24.0), 1));
            }
            timeline->cancelFrames();
        }
        
        void TimelineTest::_imageSequence()
        {
            //! \bug This uses the image sequence created by _timeline().
            auto timeline = Timeline::create(file::Path("TimelineTest.0.ppm"), _context);
            {
                std::stringstream ss;
                ss << timeline->getDuration();
                _print(ss.str());
            }
            TLR_ASSERT(otime::RationalTime(24.0, 24.0) == timeline->getDuration());
        }
    }
}
