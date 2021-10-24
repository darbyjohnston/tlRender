// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TimelinePlayerTest.h>

#include <tlrCore/AVIOSystem.h>
#include <tlrCore/Assert.h>
#include <tlrCore/TimelinePlayer.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/imageSequenceReference.h>

#include <sstream>

using namespace tlr::timeline;

namespace tlr
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
            _loopTime();
            _timelinePlayer();
        }

        void TimelinePlayerTest::_enums()
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
            const file::Path path("TimelinePlayerTest.otio");
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
            ioInfo.videoTime = clipTimeRange;
            auto write = _context->getSystem<avio::System>()->write(file::Path("TimelinePlayerTest.0.ppm"), ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipTimeRange.duration().value()); ++i)
            {
                write->writeVideo(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline player from the OTIO timeline.
            auto timelinePlayer = TimelinePlayer::create(path, _context);
            TLR_ASSERT(timelinePlayer->getTimeline());
            TLR_ASSERT(path == timelinePlayer->getPath());
            TLR_ASSERT(Options() == timelinePlayer->getOptions());
            const otime::RationalTime timelineDuration(48.0, 24.0);
            TLR_ASSERT(timelineDuration == timelinePlayer->getDuration());
            TLR_ASSERT(otime::RationalTime(10.0, 24.0) == timelinePlayer->getGlobalStartTime());
            TLR_ASSERT(imageInfo.size == timelinePlayer->getAVInfo().video[0].size);
            TLR_ASSERT(imageInfo.pixelType == timelinePlayer->getAVInfo().video[0].pixelType);
            TLR_ASSERT(timelineDuration.rate() == timelinePlayer->getDefaultSpeed());

            // Test frames.
            struct FrameOptions
            {
                uint16_t layer = 0;
                int readAhead = 100;
                int readBehind = 10;
                size_t requestCount = 16;
                size_t requestTimeout = 1;
            };
            for (const auto options : std::vector<FrameOptions>({
                FrameOptions(),
                { 1, 1, 0 } }))
            {
                timelinePlayer->setCacheReadAhead(options.readAhead);
                TLR_ASSERT(options.readAhead == timelinePlayer->getCacheReadAhead());
                timelinePlayer->setCacheReadBehind(options.readBehind);
                TLR_ASSERT(options.readBehind == timelinePlayer->getCacheReadBehind());
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
            float speed = 24.F;
            auto speedObserver = observer::ValueObserver<float>::create(
                timelinePlayer->observeSpeed(),
                [&speed](float value)
                {
                    speed = value;
                });
            const float defaultSpeed = timelinePlayer->getDefaultSpeed();
            const float doubleSpeed = defaultSpeed * 2.F;
            timelinePlayer->setSpeed(doubleSpeed);
            TLR_ASSERT(doubleSpeed == speed);
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
            otime::RationalTime currentTime = time::invalidTime;
            auto currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                timelinePlayer->observeCurrentTime(),
                [&currentTime](const otime::RationalTime& value)
                {
                    currentTime = value;
                });
            timelinePlayer->seek(otime::RationalTime(10.0, 24.0));
            TLR_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            timelinePlayer->seek(otime::RationalTime(11.0, 24.0));
            TLR_ASSERT(otime::RationalTime(11.0, 24.0) == currentTime);
            timelinePlayer->end();
            TLR_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);
            timelinePlayer->start();
            TLR_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            timelinePlayer->frameNext();
            TLR_ASSERT(otime::RationalTime(11.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FrameNextX10);
            TLR_ASSERT(otime::RationalTime(21.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FrameNextX100);
            TLR_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            timelinePlayer->framePrev();
            TLR_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FramePrevX10);
            TLR_ASSERT(otime::RationalTime(47.0, 24.0) == currentTime);
            timelinePlayer->timeAction(TimeAction::FramePrevX100);
            TLR_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);

            // Test the in/out points.
            otime::TimeRange inOutRange = time::invalidTimeRange;
            auto inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                timelinePlayer->observeInOutRange(),
                [&inOutRange](const otime::TimeRange& value)
                {
                    inOutRange = value;
                });
            timelinePlayer->setInOutRange(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(33.0, 24.0)));
            TLR_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(33.0, 24.0)) == inOutRange);
            timelinePlayer->seek(otime::RationalTime(12.0, 24.0));
            timelinePlayer->setInPoint();
            timelinePlayer->seek(otime::RationalTime(32.0, 24.0));
            timelinePlayer->setOutPoint();
            TLR_ASSERT(otime::TimeRange(otime::RationalTime(12.0, 24.0), otime::RationalTime(21.0, 24.0)) == inOutRange);
            timelinePlayer->resetInPoint();
            timelinePlayer->resetOutPoint();
            TLR_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), timelineDuration) == inOutRange);
        }
    }
}
