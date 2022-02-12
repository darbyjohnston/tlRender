// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/TimelinePlayerTest.h>

#include <tlCore/AVIOSystem.h>
#include <tlCore/Assert.h>
#include <tlCore/TimelinePlayer.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/imageSequenceReference.h>

#include <sstream>

using namespace tl::timeline;

namespace tl
{
    namespace CoreTest
    {
        TimelinePlayerTest::TimelinePlayerTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::TimelinePlayerTest", context)
        {}

        std::shared_ptr<TimelinePlayerTest> TimelinePlayerTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<TimelinePlayerTest>(new TimelinePlayerTest(context));
        }

        void TimelinePlayerTest::run()
        {
            _enums();
            _loop();
            _timelinePlayer();
        }

        void TimelinePlayerTest::_enums()
        {
            ITest::_enum<Playback>("Playback", getPlaybackEnums);
            ITest::_enum<Loop>("Loop", getLoopEnums);
            ITest::_enum<TimeAction>("TimeAction", getTimeActionEnums);
        }

        void TimelinePlayerTest::_loop()
        {
            {
                const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
                TLRENDER_ASSERT(otime::RationalTime(0.0, 24.0) == loop(otime::RationalTime(0.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(1.0, 24.0) == loop(otime::RationalTime(1.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(23.0, 24.0) == loop(otime::RationalTime(23.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(0.0, 24.0) == loop(otime::RationalTime(24.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(23.0, 24.0) == loop(otime::RationalTime(-1.0, 24.0), timeRange));
            }
            {
                const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
                auto ranges = loop(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0)), timeRange);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0)) == ranges[0]);
                ranges = loop(otime::TimeRange(otime::RationalTime(-10.0, 24.0), otime::RationalTime(34.0, 24.0)), timeRange);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0)) == ranges[0]);
                ranges = loop(otime::TimeRange(otime::RationalTime(-10.0, 24.0), otime::RationalTime(20.0, 24.0)), timeRange);
                TLRENDER_ASSERT(2 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(14.0, 24.0), otime::RationalTime(10.0, 24.0)) == ranges[0]);
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(10.0, 24.0)) == ranges[1]);
                ranges = loop(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(20.0, 24.0)), timeRange);
                TLRENDER_ASSERT(2 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(14.0, 24.0)) == ranges[0]);
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(6.0, 24.0)) == ranges[1]);
            }
            {
                const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                auto ranges = loop(otime::TimeRange(otime::RationalTime(-1.0, 24.0), otime::RationalTime(2.0, 24.0)), timeRange);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0)) == ranges[0]);
            }
        }

        void TimelinePlayerTest::_timelinePlayer()
        {
            // Write an OTIO timeline.
            auto otioTrack = new otio::Track();
            auto otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelinePlayerTest.", ".ppm", 0, 1, 1, 0));
            const otime::TimeRange clipTimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
            otioClip->set_source_range(clipTimeRange);
            otio::ErrorStatus errorStatus = otio::ErrorStatus::OK;
            otioTrack->append_child(otioClip, &errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error("Cannot append child");
            }
            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "TimelinePlayerTest.", ".ppm", 0, 1, 1, 0));
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
            auto otioTimeline = new otio::Timeline;
            otioTimeline->set_tracks(otioStack);
            otioTimeline->set_global_start_time(otime::RationalTime(10.0, 24.0));
            const std::string fileName("TimelinePlayerTest.otio");
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
            auto write = _context->getSystem<avio::System>()->write(file::Path("TimelinePlayerTest.0.ppm"), ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipTimeRange.duration().value()); ++i)
            {
                write->writeVideo(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline player from the OTIO timeline.
            auto timeline = Timeline::create(fileName, _context);
            auto timelinePlayer = TimelinePlayer::create(timeline, _context);
            TLRENDER_ASSERT(timelinePlayer->getTimeline());
            TLRENDER_ASSERT(fileName == timelinePlayer->getPath().get());
            TLRENDER_ASSERT(Options() == timelinePlayer->getOptions());
            const otime::RationalTime timelineDuration(48.0, 24.0);
            TLRENDER_ASSERT(timelineDuration == timelinePlayer->getDuration());
            TLRENDER_ASSERT(otime::RationalTime(10.0, 24.0) == timelinePlayer->getGlobalStartTime());
            TLRENDER_ASSERT(imageInfo.size == timelinePlayer->getAVInfo().video[0].size);
            TLRENDER_ASSERT(imageInfo.pixelType == timelinePlayer->getAVInfo().video[0].pixelType);
            TLRENDER_ASSERT(timelineDuration.rate() == timelinePlayer->getDefaultSpeed());

            // Test frames.
            struct FrameOptions
            {
                uint16_t layer = 0;
                otime::RationalTime readAhead = otime::RationalTime(4.0, 1.0);
                otime::RationalTime readBehind = otime::RationalTime(0.4, 1.0);
                size_t requestCount = 16;
                size_t requestTimeout = 1;
            };
            for (const auto options : std::vector<FrameOptions>({
                FrameOptions(),
                { 1, otime::RationalTime(1.0, 24.0), otime::RationalTime(0.0, 1.0) }}))
            {
                timelinePlayer->setCacheReadAhead(options.readAhead);
                TLRENDER_ASSERT(options.readAhead == timelinePlayer->observeCacheReadAhead()->get());
                timelinePlayer->setCacheReadBehind(options.readBehind);
                TLRENDER_ASSERT(options.readBehind == timelinePlayer->observeCacheReadBehind()->get());
                auto videoDataObserver = observer::ValueObserver<timeline::VideoData>::create(
                    timelinePlayer->observeVideo(),
                    [this](const timeline::VideoData& value)
                    {
                        std::stringstream ss;
                        ss << "Video time: " << value.time;
                        _print(ss.str());
                    });
                auto cachePercentageObserver = observer::ValueObserver<float>::create(
                    timelinePlayer->observeCachePercentage(),
                    [this](float value)
                    {
                        std::stringstream ss;
                        ss << "Cache: " << value << "%";
                        _print(ss.str());
                    });
                auto cachedVideoFramesObserver = observer::ListObserver<otime::TimeRange>::create(
                    timelinePlayer->observeCachedVideoFrames(),
                    [this](const std::vector<otime::TimeRange>& value)
                    {
                        std::stringstream ss;
                        ss << "Cached video frames: ";
                        for (const auto& i : value)
                        {
                            ss << i << " ";
                        }
                        _print(ss.str());
                    });
                auto cachedAudioFramesObserver = observer::ListObserver<otime::TimeRange>::create(
                    timelinePlayer->observeCachedAudioFrames(),
                    [this](const std::vector<otime::TimeRange>& value)
                    {
                        std::stringstream ss;
                        ss << "Cached audio frames: ";
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
            }
            
            // Test the playback speed.
            double speed = 24.0;
            auto speedObserver = observer::ValueObserver<double>::create(
                timelinePlayer->observeSpeed(),
                [&speed](double value)
                {
                    speed = value;
                });
            const double defaultSpeed = timelinePlayer->getDefaultSpeed();
            const double doubleSpeed = defaultSpeed * 2.0;
            timelinePlayer->setSpeed(doubleSpeed);
            TLRENDER_ASSERT(doubleSpeed == speed);
            timelinePlayer->setSpeed(defaultSpeed);

            // Test the playback mode.
            Playback playback = Playback::Stop;
            auto playbackObserver = observer::ValueObserver<Playback>::create(
                timelinePlayer->observePlayback(),
                [&playback](Playback value)
                {
                    playback = value;
                });
            timelinePlayer->setLoop(Loop::Loop);
            timelinePlayer->setPlayback(Playback::Forward);
            TLRENDER_ASSERT(Playback::Forward == playback);

            // Test the playback loop mode.
            Loop loop = Loop::Loop;
            auto loopObserver = observer::ValueObserver<Loop>::create(
                timelinePlayer->observeLoop(),
                [&loop](Loop value)
                {
                    loop = value;
                });
            timelinePlayer->setLoop(Loop::Once);
            TLRENDER_ASSERT(Loop::Once == loop);

            // Test the current time.
            timelinePlayer->setPlayback(Playback::Stop);
            otime::RationalTime currentTime = time::invalidTime;
            auto currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                timelinePlayer->observeCurrentTime(),
                [&currentTime](const otime::RationalTime& value)
                {
                    currentTime = value;
                });
            timelinePlayer->seek(otime::RationalTime(10.0, 24.0));
            TLRENDER_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            timelinePlayer->seek(otime::RationalTime(11.0, 24.0));
            TLRENDER_ASSERT(otime::RationalTime(11.0, 24.0) == currentTime);
            timelinePlayer->end();
            TLRENDER_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);
            timelinePlayer->start();
            TLRENDER_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            timelinePlayer->frameNext();
            TLRENDER_ASSERT(otime::RationalTime(11.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FrameNextX10);
            TLRENDER_ASSERT(otime::RationalTime(21.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FrameNextX100);
            TLRENDER_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            timelinePlayer->framePrev();
            TLRENDER_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FramePrevX10);
            TLRENDER_ASSERT(otime::RationalTime(47.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FramePrevX100);
            TLRENDER_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);

            // Test the in/out points.
            otime::TimeRange inOutRange = time::invalidTimeRange;
            auto inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                timelinePlayer->observeInOutRange(),
                [&inOutRange](const otime::TimeRange& value)
                {
                    inOutRange = value;
                });
            timelinePlayer->setInOutRange(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(33.0, 24.0)));
            TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(33.0, 24.0)) == inOutRange);
            timelinePlayer->seek(otime::RationalTime(12.0, 24.0));
            timelinePlayer->setInPoint();
            timelinePlayer->seek(otime::RationalTime(32.0, 24.0));
            timelinePlayer->setOutPoint();
            TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(12.0, 24.0), otime::RationalTime(21.0, 24.0)) == inOutRange);
            timelinePlayer->resetInPoint();
            timelinePlayer->resetOutPoint();
            TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), timelineDuration) == inOutRange);
        }
    }
}
