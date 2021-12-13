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
            _transitions();
            _videoData();
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
                TLR_ASSERT(toTransition(std::string()) == Transition::None);
                TLR_ASSERT(toTransition("SMPTE_Dissolve") == Transition::Dissolve);
            }
        }
        
        void TimelineTest::_videoData()
        {
            {
                VideoLayer a, b;
                TLR_ASSERT(a == b);
                a.transition = Transition::Dissolve;
                TLR_ASSERT(a != b);
            }
            {
                VideoData a, b;
                TLR_ASSERT(a == b);
                a.time = otime::RationalTime(1.0, 24.0);
                TLR_ASSERT(a != b);
            }
        }
        
        void TimelineTest::_timeline()
        {
            for (const auto& i : getExtensions(
                static_cast<int>(avio::FileExtensionType::VideoAndAudio),
                _context))
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
            const std::string fileName("TimelineTest.otio");
            otioTimeline->to_json_file(fileName, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot write file: " + fileName);
            }

            // Write the image sequence files.
            const imaging::Info imageInfo(16, 16, imaging::PixelType::RGB_U8);
            const auto image = imaging::Image::create(imageInfo);
            avio::Info ioInfo;
            ioInfo.video.push_back(imageInfo);
            ioInfo.videoTime = clipTimeRange;
            auto write = _context->getSystem<avio::System>()->write(file::Path("TimelineTest.0.ppm"), ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipTimeRange.duration().value()); ++i)
            {
                write->writeVideo(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline from the OTIO timeline.
            auto timeline = Timeline::create(fileName, _context);
            TLR_ASSERT(timeline->getTimeline());
            TLR_ASSERT(fileName == timeline->getPath().get());
            TLR_ASSERT(Options() == timeline->getOptions());
            const otime::RationalTime timelineDuration(48.0, 24.0);
            TLR_ASSERT(timelineDuration == timeline->getDuration());
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == timeline->getGlobalStartTime());
            TLR_ASSERT(imageInfo.size == timeline->getAVInfo().video[0].size);
            TLR_ASSERT(imageInfo.pixelType == timeline->getAVInfo().video[0].pixelType);

            // Get video from the timeline.
            std::vector<timeline::VideoData> videoData;
            std::vector<std::future<timeline::VideoData> > futures;
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0), 1));
            }
            while (videoData.size() < static_cast<size_t>(timelineDuration.value()) * 2)
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

            // Get video from the timeline, setting the active range.
            timeline->setVideoRanges({ otime::TimeRange(otime::RationalTime(0.0, 24.0), timelineDuration) });
            videoData.clear();
            futures.clear();
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0), 1));
            }
            while (videoData.size() < static_cast<size_t>(timelineDuration.value()) * 2)
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

            // Cancel requests.
            videoData.clear();
            futures.clear();
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
            {
                futures.push_back(timeline->getVideo(otime::RationalTime(i, 24.0), 1));
            }
            timeline->cancelRequests();
        }
        
        void TimelineTest::_imageSequence()
        {
            //! \bug This uses the image sequence created by _timeline().
            auto timeline = Timeline::create("TimelineTest.0.ppm", _context);
            {
                std::stringstream ss;
                ss << timeline->getDuration();
                _print(ss.str());
            }
            TLR_ASSERT(otime::RationalTime(24.0, 24.0) == timeline->getDuration());
        }
    }
}
