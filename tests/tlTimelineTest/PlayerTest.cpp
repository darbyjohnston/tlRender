// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/PlayerTest.h>

#include <tlTimeline/Player.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/Time.h>

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
        PlayerTest::PlayerTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::PlayerTest")
        {}

        std::shared_ptr<PlayerTest> PlayerTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<PlayerTest>(new PlayerTest(context));
        }

        void PlayerTest::run()
        {
            _enums();
            _player();
        }

        void PlayerTest::_enums()
        {
            ITest::_enum<Playback>("Playback", getPlaybackEnums);
            ITest::_enum<Loop>("Loop", getLoopEnums);
            ITest::_enum<TimeAction>("TimeAction", getTimeActionEnums);
        }

        void PlayerTest::_player()
        {
            // Test timeline players.
            const std::vector<file::Path> paths =
            {
                file::Path(TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v"),
                file::Path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg"),
                file::Path(TLRENDER_SAMPLE_DATA, "MovieAndSeq.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "TransitionGap.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClip.otioz"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClipSeq.otioz")
            };
            for (const auto& path : paths)
            {
                try
                {
                    _print(ftk::Format("Timeline: {0}").arg(path.get()));
                    auto timeline = Timeline::create(_context, path.get());
                    auto player = Player::create(_context, timeline);
                    FTK_ASSERT(player->getTimeline());
                    _player(player);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
            for (const auto& path : paths)
            {
                try
                {
                    _print(ftk::Format("Memory timeline: {0}").arg(path.get()));
                    auto otioTimeline = timeline::create(_context, path);
                    toMemoryReferences(otioTimeline, path.getDirectory(), ToMemoryReference::Shared);
                    auto timeline = Timeline::create(_context, otioTimeline);
                    auto player = Player::create(_context, timeline);
                    _player(player);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }

        void PlayerTest::_player(const std::shared_ptr<timeline::Player>& player)
        {
            const file::Path& path = player->getPath();
            const file::Path& audioPath = player->getAudioPath();
            const PlayerOptions& playerOptions = player->getPlayerOptions();
            const Options options = player->getOptions();
            const OTIO_NS::TimeRange& timeRange = player->getTimeRange();
            const io::Info& ioInfo = player->getIOInfo();
            const double defaultSpeed = player->getDefaultSpeed();
            double speed = player->getSpeed();
            _print(ftk::Format("Path: {0}").arg(path.get()));
            _print(ftk::Format("Audio path: {0}").arg(audioPath.get()));
            _print(ftk::Format("Time range: {0}").arg(timeRange));
            if (!ioInfo.video.empty())
            {
                _print(ftk::Format("Video: {0}").arg(ioInfo.video.size()));
            }
            if (ioInfo.audio.isValid())
            {
                _print(ftk::Format("Audio: {0} {1} {2}").
                    arg(ioInfo.audio.channelCount).
                    arg(ioInfo.audio.dataType).
                    arg(ioInfo.audio.sampleRate));
            }
            _print(ftk::Format("Default speed: {0}").arg(defaultSpeed));
            _print(ftk::Format("Speed: {0}").arg(speed));

            // Test the playback speed.
            auto speedObserver = ftk::ValueObserver<double>::create(
                player->observeSpeed(),
                [&speed](double value)
                {
                    speed = value;
                });
            const double doubleSpeed = defaultSpeed * 2.0;
            player->setSpeed(doubleSpeed);
            FTK_ASSERT(doubleSpeed == speed);
            player->setSpeed(defaultSpeed);

            // Test the playback mode.
            Playback playback = Playback::Stop;
            auto playbackObserver = ftk::ValueObserver<Playback>::create(
                player->observePlayback(),
                [&playback](Playback value)
                {
                    playback = value;
                });
            player->setPlayback(Playback::Forward);
            FTK_ASSERT(Playback::Forward == player->getPlayback());
            FTK_ASSERT(Playback::Forward == playback);

            // Test the playback loop mode.
            Loop loop = Loop::Loop;
            auto loopObserver = ftk::ValueObserver<Loop>::create(
                player->observeLoop(),
                [&loop](Loop value)
                {
                    loop = value;
                });
            player->setLoop(Loop::Once);
            FTK_ASSERT(Loop::Once == player->getLoop());
            FTK_ASSERT(Loop::Once == loop);

            // Test the current time.
            player->setPlayback(Playback::Stop);
            OTIO_NS::RationalTime currentTime = time::invalidTime;
            auto currentTimeObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
                player->observeCurrentTime(),
                [&currentTime](const OTIO_NS::RationalTime& value)
                {
                    currentTime = value;
                });
            player->seek(timeRange.start_time());
            FTK_ASSERT(timeRange.start_time() == player->getCurrentTime());
            FTK_ASSERT(timeRange.start_time() == currentTime);
            const double rate = timeRange.duration().rate();
            player->seek(
                timeRange.start_time() + OTIO_NS::RationalTime(1.0, rate));
            FTK_ASSERT(
                timeRange.start_time() + OTIO_NS::RationalTime(1.0, rate) ==
                currentTime);
            player->gotoEnd();
            FTK_ASSERT(timeRange.end_time_inclusive() == currentTime);
            player->gotoStart();
            FTK_ASSERT(timeRange.start_time() == currentTime);
            player->frameNext();
            FTK_ASSERT(
                timeRange.start_time() + OTIO_NS::RationalTime(1.0, rate) ==
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
            OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
            auto inOutRangeObserver = ftk::ValueObserver<OTIO_NS::TimeRange>::create(
                player->observeInOutRange(),
                [&inOutRange](const OTIO_NS::TimeRange& value)
                {
                    inOutRange = value;
                });
            player->setInOutRange(OTIO_NS::TimeRange(
                timeRange.start_time(),
                OTIO_NS::RationalTime(10.0, rate)));
            FTK_ASSERT(OTIO_NS::TimeRange(
                timeRange.start_time(),
                OTIO_NS::RationalTime(10.0, rate)) == player->getInOutRange());
            FTK_ASSERT(OTIO_NS::TimeRange(
                timeRange.start_time(),
                OTIO_NS::RationalTime(10.0, rate)) == inOutRange);
            player->seek(timeRange.start_time() + OTIO_NS::RationalTime(1.0, rate));
            player->setInPoint();
            player->seek(timeRange.start_time() + OTIO_NS::RationalTime(10.0, rate));
            player->setOutPoint();
            FTK_ASSERT(OTIO_NS::TimeRange(
                timeRange.start_time() + OTIO_NS::RationalTime(1.0, rate),
                OTIO_NS::RationalTime(10.0, rate)) == inOutRange);
            player->resetInPoint();
            player->resetOutPoint();
            FTK_ASSERT(OTIO_NS::TimeRange(timeRange.start_time(), timeRange.duration()) == inOutRange);

            // Test the I/O options.
            io::Options ioOptions;
            auto ioOptionsObserver = ftk::ValueObserver<io::Options>::create(
                player->observeIOOptions(),
                [&ioOptions](const io::Options& value)
                {
                    ioOptions = value;
                });
            io::Options ioOptions2;
            ioOptions2["Layer"] = "1";
            player->setIOOptions(ioOptions2);
            FTK_ASSERT(ioOptions2 == player->getIOOptions());
            FTK_ASSERT(ioOptions2 == ioOptions);
            player->setIOOptions({});

            // Test the video layers.
            int videoLayer = 0;
            std::vector<int> compareVideoLayers;
            auto videoLayerObserver = ftk::ValueObserver<int>::create(
                player->observeVideoLayer(),
                [&videoLayer](int value)
                {
                    videoLayer = value;
                });
            auto compareVideoLayersObserver = ftk::ListObserver<int>::create(
                player->observeCompareVideoLayers(),
                [&compareVideoLayers](const std::vector<int>& value)
                {
                    compareVideoLayers = value;
                });
            int videoLayer2 = 1;
            player->setVideoLayer(videoLayer2);
            FTK_ASSERT(videoLayer2 == player->getVideoLayer());
            FTK_ASSERT(videoLayer2 == videoLayer);
            std::vector<int> compareVideoLayers2 = { 2, 3 };
            player->setCompareVideoLayers(compareVideoLayers2);
            FTK_ASSERT(compareVideoLayers2 == player->getCompareVideoLayers());
            FTK_ASSERT(compareVideoLayers2 == compareVideoLayers);
            player->setVideoLayer(0);
            player->setCompareVideoLayers({});

            // Test audio.
            float volume = 1.F;
            auto volumeObserver = ftk::ValueObserver<float>::create(
                player->observeVolume(),
                [&volume](float value)
                {
                    volume = value;
                });
            player->setVolume(.5F);
            FTK_ASSERT(.5F == player->getVolume());
            FTK_ASSERT(.5F == volume);
            player->setVolume(1.F);

            bool mute = false;
            auto muteObserver = ftk::ValueObserver<bool>::create(
                player->observeMute(),
                [&mute](bool value)
                {
                    mute = value;
                });
            player->setMute(true);
            FTK_ASSERT(player->isMuted());
            FTK_ASSERT(mute);
            player->setMute(false);

            std::vector<bool> channelMute = { false, false };
            auto channelMuteObserver = ftk::ListObserver<bool>::create(
                player->observeChannelMute(),
                [&channelMute](const std::vector<bool>& value)
                {
                    channelMute = value;
                });
            player->setChannelMute({ true, true });
            FTK_ASSERT(player->getChannelMute() == std::vector<bool>({ true, true }));
            FTK_ASSERT(channelMute[0]);
            FTK_ASSERT(channelMute[1]);
            player->setChannelMute({ false, false });

            double audioOffset = 0.0;
            auto audioOffsetObserver = ftk::ValueObserver<double>::create(
                player->observeAudioOffset(),
                [&audioOffset](double value)
                {
                    audioOffset = value;
                });
            player->setAudioOffset(0.5);
            FTK_ASSERT(0.5 == player->getAudioOffset());
            FTK_ASSERT(0.5 == audioOffset);
            player->setAudioOffset(0.0);
            
            // Test frames.
            {
                PlayerCacheOptions cacheOptions;
                auto cacheOptionsObserver = ftk::ValueObserver<PlayerCacheOptions>::create(
                    player->observeCacheOptions(),
                    [&cacheOptions](const PlayerCacheOptions& value)
                    {
                        cacheOptions = value;
                    });
                cacheOptions.videoGB = 1.F;
                player->setCacheOptions(cacheOptions);
                FTK_ASSERT(cacheOptions == player->getCacheOptions());

                auto currentVideoObserver = ftk::ListObserver<timeline::VideoData>::create(
                    player->observeCurrentVideo(),
                    [this](const std::vector<timeline::VideoData>& value)
                    {
                        std::stringstream ss;
                        ss << "Video time: ";
                        if (!value.empty())
                        {
                            ss << value.front().time;
                        }
                        _print(ss.str());
                    });
                auto currentAudioObserver = ftk::ListObserver<timeline::AudioData>::create(
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
                auto cacheInfoObserver = ftk::ValueObserver<PlayerCacheInfo>::create(
                    player->observeCacheInfo(),
                    [this](const PlayerCacheInfo& value)
                    {
                        {
                            std::stringstream ss;
                            ss << "Video/audio cached frames: " << value.video.size() << "/" << value.audio.size();
                            _print(ss.str());
                        }
                    });

                for (const auto& loop : getLoopEnums())
                {
                    player->seek(timeRange.start_time());
                    player->setLoop(loop);
                    player->setPlayback(Playback::Forward);
                    auto t = std::chrono::steady_clock::now();
                    std::chrono::duration<float> diff;
                    do
                    {
                        player->tick();
                        ftk::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);

                    player->seek(timeRange.end_time_inclusive());
                    t = std::chrono::steady_clock::now();
                    do
                    {
                        player->tick();
                        ftk::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);

                    player->seek(timeRange.end_time_inclusive());
                    player->setPlayback(Playback::Reverse);
                    t = std::chrono::steady_clock::now();
                    do
                    {
                        player->tick();
                        ftk::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);

                    player->seek(timeRange.start_time());
                    player->setSpeed(doubleSpeed);
                    t = std::chrono::steady_clock::now();
                    do
                    {
                        player->tick();
                        ftk::sleep(std::chrono::milliseconds(10));
                        const auto t2 = std::chrono::steady_clock::now();
                        diff = t2 - t;
                    } while (diff.count() < 1.F);
                    player->setSpeed(defaultSpeed);
                }
                player->setPlayback(Playback::Stop);
                player->clearCache();
            }
        }
    }
}
