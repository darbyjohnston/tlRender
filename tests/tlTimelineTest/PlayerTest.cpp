// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/PlayerTest.h>

#include <tlTimeline/Player.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>

#include <sstream>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        PlayerTest::PlayerTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::PlayerTest", context)
        {}

        std::shared_ptr<PlayerTest> PlayerTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PlayerTest>(new PlayerTest(context));
        }

        void PlayerTest::run()
        {
            _enums();
            _loop();
            _player();
        }

        void PlayerTest::_enums()
        {
            ITest::_enum<Playback>("Playback", getPlaybackEnums);
            ITest::_enum<Loop>("Loop", getLoopEnums);
            ITest::_enum<TimeAction>("TimeAction", getTimeActionEnums);
        }

        void PlayerTest::_loop()
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

        void PlayerTest::_player()
        {
            // Test timeline players.
            const std::vector<file::Path> paths =
            {
                //file::Path(TLRENDER_SAMPLE_DATA, "AudioTones.otio"),
                //file::Path(TLRENDER_SAMPLE_DATA, "AudioTonesAndVideo.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "Gap.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "MovieAndSeq.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "TransitionOverlay.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClip.otioz")
            };
            for (const auto& path : paths)
            {
                auto timeline = Timeline::create(path, _context);
                auto player = Player::create(timeline, _context);
                TLRENDER_ASSERT(player->getTimeline());
                TLRENDER_ASSERT(path == player->getPath());
                _player(player);
            }
            for (const auto& path : paths)
            {
                auto otioTimeline = timeline::create(path, _context);
                TLRENDER_ASSERT(otioTimeline);
                toMemoryReferences(otioTimeline, path.getDirectory());
                auto timeline = Timeline::create(otioTimeline, _context);
                auto player = Player::create(timeline, _context);
                TLRENDER_ASSERT(player->getTimeline());
                TLRENDER_ASSERT(path == player->getPath());
                _player(player);
            }
        }

        void PlayerTest::_player(const std::shared_ptr<timeline::Player>& player)
        {
            const otime::TimeRange& timeRange = player->getTimeRange();
            const file::Path& audioPath = player->getAudioPath();
            const PlayerOptions& playerOptions = player->getPlayerOptions();
            const Options options = player->getOptions();
            const io::Info& ioInfo = player->getIOInfo();

            // Test frames.
            struct FrameOptions
            {
                uint16_t layer = 0;
                PlayerCacheOptions cache;
                size_t requestCount = 16;
                size_t requestTimeout = 1;
            };
            FrameOptions frameOptions2;
            frameOptions2.layer = 1;
            frameOptions2.cache.readAhead = otime::RationalTime(1.0, 24.0);
            frameOptions2.cache.readBehind = otime::RationalTime(0.0, 1.0);
            for (const auto options : std::vector<FrameOptions>({ FrameOptions(), frameOptions2 }))
            {
                player->setCacheOptions(options.cache);
                TLRENDER_ASSERT(options.cache == player->observeCacheOptions()->get());
                auto currentVideoObserver = observer::ValueObserver<timeline::VideoData>::create(
                    player->observeCurrentVideo(),
                    [this](const timeline::VideoData& value)
                    {
                        std::stringstream ss;
                        ss << "Video time: " << value.time;
                        _print(ss.str());
                    });
                auto currentAudioObserver = observer::ListObserver<timeline::AudioData>::create(
                    player->observeCurrentAudio(),
                    [this](const std::vector<timeline::AudioData>& value)
                    {
                        for (const auto& i : value)
                        {
                            std::stringstream ss;
                            ss << "Audio time: " << i.seconds;
                            _print(ss.str());
                        }
                    });
                auto cacheInfoObserver = observer::ValueObserver<PlayerCacheInfo>::create(
                    player->observeCacheInfo(),
                    [this](const PlayerCacheInfo& value)
                    {
                        {
                            std::stringstream ss;
                            ss << "Video/audio cached frames: " << value.videoFrames.size() << "/" << value.audioFrames.size();
                            _print(ss.str());
                        }
                    });
                for (const auto& loop : getLoopEnums())
                {
                    player->seek(timeRange.start_time());
                    player->setLoop(loop);
                    player->setPlayback(Playback::Forward);
                    for (size_t i = 0; i < timeRange.duration().rate(); ++i)
                    {
                        player->tick();
                        time::sleep(std::chrono::milliseconds(1));
                    }
                    player->seek(timeRange.start_time());
                    player->setPlayback(Playback::Reverse);
                    for (size_t i = 0; i < timeRange.duration().rate(); ++i)
                    {
                        player->tick();
                        time::sleep(std::chrono::milliseconds(1));
                    }
                }
                player->setPlayback(Playback::Stop);
            }

            // Test the playback speed.
            double speed = player->getSpeed();
            auto speedObserver = observer::ValueObserver<double>::create(
                player->observeSpeed(),
                [&speed](double value)
                {
                    speed = value;
                });
            const double defaultSpeed = player->getDefaultSpeed();
            const double doubleSpeed = defaultSpeed * 2.0;
            player->setSpeed(doubleSpeed);
            TLRENDER_ASSERT(doubleSpeed == speed);
            player->setSpeed(defaultSpeed);

            // Test the playback mode.
            Playback playback = Playback::Stop;
            auto playbackObserver = observer::ValueObserver<Playback>::create(
                player->observePlayback(),
                [&playback](Playback value)
                {
                    playback = value;
                });
            player->setPlayback(Playback::Forward);
            TLRENDER_ASSERT(Playback::Forward == player->getPlayback());
            TLRENDER_ASSERT(Playback::Forward == playback);

            // Test the playback loop mode.
            Loop loop = Loop::Loop;
            auto loopObserver = observer::ValueObserver<Loop>::create(
                player->observeLoop(),
                [&loop](Loop value)
                {
                    loop = value;
                });
            player->setLoop(Loop::Once);
            TLRENDER_ASSERT(Loop::Once == player->getLoop());
            TLRENDER_ASSERT(Loop::Once == loop);

            // Test the current time.
            player->setPlayback(Playback::Stop);
            otime::RationalTime currentTime = time::invalidTime;
            auto currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                player->observeCurrentTime(),
                [&currentTime](const otime::RationalTime& value)
                {
                    currentTime = value;
                });
            player->seek(timeRange.start_time());
            TLRENDER_ASSERT(timeRange.start_time() == player->getCurrentTime());
            TLRENDER_ASSERT(timeRange.start_time() == currentTime);
            const double rate = timeRange.duration().rate();
            player->seek(
                timeRange.start_time() + otime::RationalTime(1.0, rate));
            TLRENDER_ASSERT(
                timeRange.start_time() + otime::RationalTime(1.0, rate) ==
                currentTime);
            player->end();
            TLRENDER_ASSERT(timeRange.end_time_inclusive() == currentTime);
            player->start();
            TLRENDER_ASSERT(timeRange.start_time() == currentTime);
            player->frameNext();
            TLRENDER_ASSERT(
                timeRange.start_time() + otime::RationalTime(1.0, rate) ==
                currentTime);
            player->timeAction(TimeAction::FrameNextX10);
            player->timeAction(TimeAction::FrameNextX100);
            player->framePrev();
            player->timeAction(TimeAction::FramePrevX10);
            player->timeAction(TimeAction::FramePrevX100);
            player->timeAction(TimeAction::JumpForward1s);
            player->timeAction(TimeAction::JumpForward10s);
            player->timeAction(TimeAction::JumpBack1s);
            player->timeAction(TimeAction::JumpBack10s);

            // Test the in/out points.
            otime::TimeRange inOutRange = time::invalidTimeRange;
            auto inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                player->observeInOutRange(),
                [&inOutRange](const otime::TimeRange& value)
                {
                    inOutRange = value;
                });
            player->setInOutRange(otime::TimeRange(
                timeRange.start_time(),
                otime::RationalTime(10.0, rate)));
            TLRENDER_ASSERT(otime::TimeRange(
                timeRange.start_time(),
                otime::RationalTime(10.0, rate)) == player->getInOutRange());
            TLRENDER_ASSERT(otime::TimeRange(
                timeRange.start_time(),
                otime::RationalTime(10.0, rate)) == inOutRange);
            player->seek(timeRange.start_time() + otime::RationalTime(1.0, rate));
            player->setInPoint();
            player->seek(timeRange.start_time() + otime::RationalTime(10.0, rate));
            player->setOutPoint();
            TLRENDER_ASSERT(otime::TimeRange(
                timeRange.start_time() + otime::RationalTime(1.0, rate),
                otime::RationalTime(10.0, rate)) == inOutRange);
            player->resetInPoint();
            player->resetOutPoint();
            TLRENDER_ASSERT(otime::TimeRange(timeRange.start_time(), timeRange.duration()) == inOutRange);
        }
    }
}
