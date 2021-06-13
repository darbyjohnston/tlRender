// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TimelinePlayerTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/IO.h>
#include <tlrCore/TimelinePlayer.h>

#include <opentimelineio/timeline.h>
#include <opentimelineio/imageSequenceReference.h>

#include <sstream>

using namespace tlr::timeline;

namespace tlr
{
    namespace CoreTest
    {
        TimelinePlayerTest::TimelinePlayerTest() :
            ITest("CoreTest::TimelinePlayerTest")
        {}

        std::shared_ptr<TimelinePlayerTest> TimelinePlayerTest::create()
        {
            return std::shared_ptr<TimelinePlayerTest>(new TimelinePlayerTest);
        }

        void TimelinePlayerTest::run()
        {
            _enum();
            _loopTime();
            _fitWindow();
            _timelinePlayer();
        }

        void TimelinePlayerTest::_enum()
        {
            ITest::_enum<Playback>("Playback", getPlaybackEnums);
            ITest::_enum<Loop>("Loop", getLoopEnums);
            ITest::_enum<TimeAction>("TimeAction", getTimeActionEnums);
        }

        void TimelinePlayerTest::_loopTime()
        {
            const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == loopTime(otime::RationalTime(0.0, 24.0), timeRange));
            TLR_ASSERT(otime::RationalTime(1.0, 24.0) == loopTime(otime::RationalTime(1.0, 24.0), timeRange));
            TLR_ASSERT(otime::RationalTime(23.0, 24.0) == loopTime(otime::RationalTime(23.0, 24.0), timeRange));
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == loopTime(otime::RationalTime(24.0, 24.0), timeRange));
            TLR_ASSERT(otime::RationalTime(23.0, 24.0) == loopTime(otime::RationalTime(-1.0, 24.0), timeRange));
        }

        void TimelinePlayerTest::_fitWindow()
        {
            TLR_ASSERT(math::BBox2f(0.F, 0.F, 100.F, 100.F) == fitWindow(imaging::Size(100, 100), imaging::Size(100, 100)));
            TLR_ASSERT(math::BBox2f(50.F, 0.F, 100.F, 100.F) == fitWindow(imaging::Size(100, 100), imaging::Size(200, 100)));
            TLR_ASSERT(math::BBox2f(0.F, 50.F, 100.F, 100.F) == fitWindow(imaging::Size(100, 100), imaging::Size(100, 200)));
        }

        void TimelinePlayerTest::_timelinePlayer()
        {
            // Write an OTIO timeline.
            auto otioTrack = new otio::Track();
            auto otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelinePlayerTest.", ".png", 0, 1, 1, 0));
            const otime::RationalTime clipDuration(24.0, 24.0);
            otioClip->set_source_range(otime::TimeRange(otime::RationalTime(0.0, 24.0), clipDuration));
            otio::ErrorStatus errorStatus = otio::ErrorStatus::OK;
            otioTrack->append_child(otioClip, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot append child");
            }
            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelinePlayerTest.", ".png", 0, 1, 1, 0));
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
            std::string fileName = "TimelinePlayerTest.otio";
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
            auto write = ioSystem->write("TimelinePlayerTest.0.png", ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipDuration.value()); ++i)
            {
                write->writeVideoFrame(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline player from the OTIO timeline.
            auto timelinePlayer = TimelinePlayer::create(fileName);
            TLR_ASSERT(fileName == timelinePlayer->getFileName());
            const otime::RationalTime timelineDuration(48.0, 24.0);
            TLR_ASSERT(timelineDuration == timelinePlayer->getDuration());
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == timelinePlayer->getGlobalStartTime());
            TLR_ASSERT(imageInfo == timelinePlayer->getImageInfo());
            TLR_ASSERT(std::vector<otime::TimeRange>(
                {
                    otime::TimeRange(otime::RationalTime(0.0, 24.0), clipDuration),
                    otime::TimeRange(otime::RationalTime(24.0, 24.0), clipDuration)
                }) == timelinePlayer->getClipRanges());

            // Test frames.
            timelinePlayer->setFrameCacheReadAhead(10);
            TLR_ASSERT(10 == timelinePlayer->getFrameCacheReadAhead());
            timelinePlayer->setFrameCacheReadBehind(1);
            TLR_ASSERT(1 == timelinePlayer->getFrameCacheReadBehind());
            auto frameObserver = observer::ValueObserver<io::VideoFrame>::create(
                timelinePlayer->observeFrame(),
                [this](const io::VideoFrame& value)
                {
                    if (value.image)
                    {
                        std::stringstream ss;
                        ss << "Frame: " << value.time;
                        _print(ss.str());
                    }
                });
            auto cachedFramesObserver = observer::ListObserver<otime::TimeRange>::create(
                timelinePlayer->observeCachedFrames(),
                [this](const std::vector<otime::TimeRange>& value)
                {
                    std::stringstream ss;
                    ss << "Cached frames: ";
                    for (const auto& i : value)
                    {
                        ss << i << " ";
                    }
                    _print(ss.str());
                });
            for (const auto& loop : getLoopEnums())
            {
                timelinePlayer->setLoop(loop);
                timelinePlayer->setPlayback(Playback::Forward);
                for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
                {
                    timelinePlayer->tick();
                    time::sleep(std::chrono::microseconds(1000000 / 24));
                }
                timelinePlayer->setPlayback(Playback::Reverse);
                for (size_t i = 0; i < static_cast<size_t>(timelineDuration.value()); ++i)
                {
                    timelinePlayer->tick();
                    time::sleep(std::chrono::microseconds(1000000 / 24));
                }
            }
            timelinePlayer->setPlayback(Playback::Stop);

            // Test the playback mode.
            Playback playback = Playback::Stop;
            auto playbackObserver = observer::ValueObserver<Playback>::create(
                timelinePlayer->observePlayback(),
                [&playback](Playback value)
                {
                    playback = value;
                });
            timelinePlayer->setPlayback(Playback::Forward);
            TLR_ASSERT(Playback::Forward == playback);

            // Test the playback loop mode.
            Loop loop = Loop::Loop;
            auto loopObserver = observer::ValueObserver<Loop>::create(
                timelinePlayer->observeLoop(),
                [&loop](Loop value)
                {
                    loop = value;
                });
            timelinePlayer->setLoop(Loop::Once);
            TLR_ASSERT(Loop::Once == loop);

            // Test the current time.
            timelinePlayer->setPlayback(Playback::Stop);
            otime::RationalTime currentTime = invalidTime;
            auto currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                timelinePlayer->observeCurrentTime(),
                [&currentTime](const otime::RationalTime& value)
                {
                    currentTime = value;
                });
            timelinePlayer->seek(otime::RationalTime(0.0, 24.0));
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == currentTime);
            timelinePlayer->seek(otime::RationalTime(1.0, 24.0));
            TLR_ASSERT(otime::RationalTime(1.0, 24.0) == currentTime);
            timelinePlayer->end();
            TLR_ASSERT(otime::RationalTime(47.0, 24.0) == currentTime);
            timelinePlayer->start();
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == currentTime);
            timelinePlayer->frameNext();
            TLR_ASSERT(otime::RationalTime(1.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FrameNextX10);
            TLR_ASSERT(otime::RationalTime(11.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FrameNextX100);
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == currentTime);
            timelinePlayer->framePrev();
            TLR_ASSERT(otime::RationalTime(47.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FramePrevX10);
            TLR_ASSERT(otime::RationalTime(37.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FramePrevX100);
            TLR_ASSERT(otime::RationalTime(47.0, 24.0) == currentTime);
            timelinePlayer->clipPrev();
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == currentTime);
            timelinePlayer->clipPrev();
            TLR_ASSERT(otime::RationalTime(24.0, 24.0) == currentTime);
            timelinePlayer->clipNext();
            TLR_ASSERT(otime::RationalTime(0.0, 24.0) == currentTime);
            timelinePlayer->clipNext();
            TLR_ASSERT(otime::RationalTime(24.0, 24.0) == currentTime);

            // Test the in/out points.
            otime::TimeRange inOutRange = invalidTimeRange;
            auto inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                timelinePlayer->observeInOutRange(),
                [&inOutRange](const otime::TimeRange& value)
                {
                    inOutRange = value;
                });
            timelinePlayer->setInOutRange(otime::TimeRange(otime::RationalTime(1.0, 24.0), otime::RationalTime(23.0, 24.0)));
            TLR_ASSERT(otime::TimeRange(otime::RationalTime(1.0, 24.0), otime::RationalTime(23.0, 24.0)) == inOutRange);
            timelinePlayer->seek(otime::RationalTime(2.0, 24.0));
            timelinePlayer->setInPoint();
            timelinePlayer->seek(otime::RationalTime(22.0, 24.0));
            timelinePlayer->setOutPoint();
            TLR_ASSERT(otime::TimeRange(otime::RationalTime(2.0, 24.0), otime::RationalTime(21.0, 24.0)) == inOutRange);
            timelinePlayer->resetInPoint();
            timelinePlayer->resetOutPoint();
            TLR_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), timelineDuration) == inOutRange);
        }
    }
}
