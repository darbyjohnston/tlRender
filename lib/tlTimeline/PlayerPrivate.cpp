// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <dtk/core/Context.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace timeline
    {
        OTIO_NS::RationalTime Player::Private::loopPlayback(const OTIO_NS::RationalTime& time, bool& looped)
        {
            OTIO_NS::RationalTime out = time;
            looped = false;

            const auto& range = inOutRange->get();
            switch (loop->get())
            {
            case Loop::Loop:
            {
                out = timeline::loop(out, range, &looped);
                if (looped)
                {
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioReset(out);
                    }
                    if (!hasAudio())
                    {
                        playbackReset(out);
                    }
                }
                break;
            }
            case Loop::Once:
            {
                const auto playbackValue = playback->get();
                if (out < range.start_time() && Playback::Reverse == playbackValue)
                {
                    out = range.start_time();
                    playback->setIfChanged(Playback::Stop);
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.state.playback = Playback::Stop;
                        mutex.clearRequests = true;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.state.playback = Playback::Stop;
                    }
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Stop);
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.state.playback = Playback::Stop;
                        mutex.clearRequests = true;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.state.playback = Playback::Stop;
                    }
                }
                break;
            }
            case Loop::PingPong:
            {
                const auto playbackValue = playback->get();
                if (out < range.start_time() && Playback::Reverse == playbackValue)
                {
                    out = range.start_time();
                    playback->setIfChanged(Playback::Forward);
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.state.playback = Playback::Forward;
                        mutex.state.currentTime = out;
                        mutex.clearRequests = true;
                        mutex.cacheDirection = CacheDirection::Forward;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.state.playback = Playback::Forward;
                        audioReset(out);
                    }
                    if (!hasAudio())
                    {
                        playbackReset(out);
                    }
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Reverse);
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.state.playback = Playback::Reverse;
                        mutex.state.currentTime = out;
                        mutex.clearRequests = true;
                        mutex.cacheDirection = CacheDirection::Reverse;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.state.playback = Playback::Reverse;
                        audioReset(out);
                    }
                    if (!hasAudio())
                    {
                        playbackReset(out);
                    }
                }
                break;
            }
            default: break;
            }

            return out;
        }

        void Player::Private::clearRequests()
        {
            std::vector<std::vector<uint64_t> > ids(1 + thread.state.compare.size());
            for (const auto& i : thread.videoDataRequests)
            {
                for (size_t j = 0; j < i.second.list.size() && j < ids.size(); ++j)
                {
                    ids[j].push_back(i.second.list[j].id);
                }
            }
            for (const auto& i : thread.audioDataRequests)
            {
                ids[0].push_back(i.second.request.id);
            }
            timeline->cancelRequests(ids[0]);
            for (size_t i = 0; i < thread.state.compare.size(); ++i)
            {
                thread.state.compare[i]->cancelRequests(ids[i + 1]);
            }
            thread.videoDataRequests.clear();
            thread.audioDataRequests.clear();
        }

        void Player::Private::clearCache()
        {
            thread.videoCache.clear();
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                mutex.cacheInfo = PlayerCacheInfo();
            }
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.cache.clear();
            }
        }

        void Player::Private::cacheUpdate()
        {
            //std::cout << "current time: " << currentTime->get() << std::endl;

            thread.videoCache.setMax(thread.state.cacheOptions.videoGB * dtk::gigabyte);
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.cache.setMax(thread.state.cacheOptions.audioGB * dtk::gigabyte);
            }

            // Fill the video cache.
            if (!ioInfo.video.empty())
            {
                const int64_t duration = thread.state.inOutRange.duration().value();
                const int64_t readBehind = OTIO_NS::RationalTime(
                    thread.state.cacheOptions.readBehind, 1.0).rescaled_to(thread.state.currentTime.rate()).value();
                for (;
                    thread.videoFillFrame < duration &&
                    thread.videoDataRequests.size() < playerOptions.videoRequestMax;
                    ++thread.videoFillFrame)
                {
                    const OTIO_NS::RationalTime offset(
                        thread.cacheDirection == CacheDirection::Forward ?
                        (thread.videoFillFrame - readBehind) :
                        (readBehind - thread.videoFillFrame),
                        thread.state.currentTime.rate());
                    const OTIO_NS::RationalTime t = timeline::loop(
                        thread.state.currentTime + offset,
                        thread.state.inOutRange);
                    size_t byteCount = timeline->getVideoSize(t).future.get();
                    for (size_t i = 0; i < thread.state.compare.size(); ++i)
                    {
                        const OTIO_NS::RationalTime t2 = timeline::getCompareTime(
                            t,
                            timeRange,
                            thread.state.compare[i]->getTimeRange(),
                            thread.state.compareTime);
                        byteCount += thread.state.compare[i]->getVideoSize(t2).future.get();
                    }
                    thread.videoFillByteCount += byteCount;
                    if (thread.videoFillByteCount >= thread.videoCache.getMax())
                    {
                        break;
                    }
                    if (!thread.videoCache.contains(t))
                    {
                        const auto i = thread.videoDataRequests.find(t);
                        if (i == thread.videoDataRequests.end())
                        {
                            //std::cout << this << " video request: " << t << std::endl;
                            auto& request = thread.videoDataRequests[t];
                            io::Options ioOptions2 = thread.state.ioOptions;
                            ioOptions2["Layer"] = dtk::Format("{0}").arg(thread.state.videoLayer);
                            request.list.clear();
                            request.list.push_back(timeline->getVideo(t, ioOptions2));
                            for (size_t i = 0; i < thread.state.compare.size(); ++i)
                            {
                                const OTIO_NS::RationalTime time2 = timeline::getCompareTime(
                                    t,
                                    timeRange,
                                    thread.state.compare[i]->getTimeRange(),
                                    thread.state.compareTime);
                                ioOptions2["Layer"] = dtk::Format("{0}").
                                    arg(i < thread.state.compareVideoLayers.size() ?
                                        thread.state.compareVideoLayers[i] :
                                        thread.state.videoLayer);
                                request.list.push_back(thread.state.compare[i]->getVideo(time2, ioOptions2));
                            }
                            request.byteCount = byteCount;
                        }
                    }
                }
            }

            // Fill the audio cache.
            if (ioInfo.audio.isValid())
            {
                size_t cacheMax = 0;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    cacheMax = audioMutex.cache.getMax();
                }
                const int64_t duration = thread.state.inOutRange.duration().rescaled_to(1.0).value();
                for (;
                    thread.audioFillSeconds < duration &&
                    thread.audioDataRequests.size() < playerOptions.audioRequestMax;
                    ++thread.audioFillSeconds)
                {
                    const double offset =
                        thread.cacheDirection == CacheDirection::Forward ?
                        (thread.audioFillSeconds - thread.state.cacheOptions.readBehind) :
                        (thread.state.cacheOptions.readBehind - thread.audioFillSeconds);
                    const int64_t t = timeline::loop(
                        thread.state.currentTime.rescaled_to(1.0).value() + offset + thread.state.audioOffset,
                        thread.state.inOutRange);
                    const size_t byteCount = timeline->getAudioSize(t).future.get();
                    thread.audioFillByteCount += byteCount;
                    if (thread.audioFillByteCount >= cacheMax)
                    {
                        break;
                    }
                    bool found = false;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        found = audioMutex.cache.contains(t);
                    }
                    if (!found)
                    {
                        const auto i = thread.audioDataRequests.find(t);
                        if (i == thread.audioDataRequests.end())
                        {
                            auto& request = thread.audioDataRequests[t];
                            request.byteCount = byteCount;
                            request.request = timeline->getAudio(t, thread.state.ioOptions);
                        }
                    }
                }
            }

            // Check for finished video.
            auto videoDataRequestsIt = thread.videoDataRequests.begin();
            while (videoDataRequestsIt != thread.videoDataRequests.end())
            {
                bool ready = true;
                for (auto videoDataRequestIt = videoDataRequestsIt->second.list.begin();
                    videoDataRequestIt != videoDataRequestsIt->second.list.end();
                    ++videoDataRequestIt)
                {
                    ready &= videoDataRequestIt->future.valid() &&
                        videoDataRequestIt->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                }
                if (ready)
                {
                    const OTIO_NS::RationalTime time = videoDataRequestsIt->first;
                    std::vector<VideoData> videoDataList;
                    for (auto videoDataRequestIt = videoDataRequestsIt->second.list.begin();
                        videoDataRequestIt != videoDataRequestsIt->second.list.end();
                        ++videoDataRequestIt)
                    {
                        auto videoData = videoDataRequestIt->future.get();
                        videoData.time = time;
                        videoDataList.emplace_back(videoData);
                    }
                    thread.videoCache.add(time, videoDataList, videoDataRequestsIt->second.byteCount);
                    videoDataRequestsIt = thread.videoDataRequests.erase(videoDataRequestsIt);
                }
                else
                {
                    ++videoDataRequestsIt;
                }
            }

            // Check for finished audio.
            auto audioDataRequestsIt = thread.audioDataRequests.begin();
            while (audioDataRequestsIt != thread.audioDataRequests.end())
            {
                if (audioDataRequestsIt->second.request.future.valid() &&
                    audioDataRequestsIt->second.request.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto audioData = audioDataRequestsIt->second.request.future.get();
                    audioData.seconds = audioDataRequestsIt->first;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.cache.add(
                            audioDataRequestsIt->first,
                            audioData,
                            audioDataRequestsIt->second.byteCount);
                    }
                    audioDataRequestsIt = thread.audioDataRequests.erase(audioDataRequestsIt);
                }
                else
                {
                    ++audioDataRequestsIt;
                }
            }

            // Prioritize cached video frames.
            {
                std::list<OTIO_NS::RationalTime> times;
                const int64_t duration = thread.state.inOutRange.duration().value();
                const int64_t readBehind = OTIO_NS::RationalTime(
                    thread.state.cacheOptions.readBehind, 1.0).rescaled_to(thread.state.currentTime.rate()).value();
                size_t fillByteCount = 0;
                for (int64_t fillFrame = 0; fillFrame < duration; ++fillFrame)
                {
                    const OTIO_NS::RationalTime offset(
                        thread.cacheDirection == CacheDirection::Forward ?
                        (fillFrame - readBehind) :
                        (readBehind - fillFrame),
                        thread.state.currentTime.rate());
                    const OTIO_NS::RationalTime t = timeline::loop(
                        thread.state.currentTime + offset,
                        thread.state.inOutRange);
                    fillByteCount += timeline->getVideoSize(t).future.get();
                    if (fillByteCount >= thread.videoCache.getMax())
                    {
                        break;
                    }
                    times.push_back(t);
                }
                times.reverse();
                for (const auto t : times)
                {
                    thread.videoCache.touch(t);
                }
            }

            // Prioritize cached audio frames.
            {
                std::list<int64_t> times;
                size_t cacheMax = 0;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    cacheMax = audioMutex.cache.getMax();
                }
                const int64_t duration = thread.state.inOutRange.duration().rescaled_to(1.0).value();
                size_t fillByteCount = 0;
                for (int64_t fillSeconds = 0; fillSeconds < duration; ++fillSeconds)
                {
                    const double offset =
                        thread.cacheDirection == CacheDirection::Forward ?
                        (fillSeconds - thread.state.cacheOptions.readBehind) :
                        (thread.state.cacheOptions.readBehind - fillSeconds);
                    const int64_t t = timeline::loop(
                        thread.state.currentTime.rescaled_to(1.0).value() + offset + thread.state.audioOffset,
                        thread.state.inOutRange);
                    fillByteCount += timeline->getAudioSize(t).future.get();
                    if (fillByteCount >= cacheMax)
                    {
                        break;
                    }
                    times.push_back(t);
                }
                times.reverse();
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                for (const auto t : times)
                {
                    audioMutex.cache.touch(t);
                }
            }

            // Update cache information.
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - thread.cacheTimer;
            if (diff.count() > .5F)
            {
                thread.cacheTimer = now;
                std::vector<OTIO_NS::RationalTime> cachedVideoFrames;
                for (const auto& key : thread.videoCache.getKeys())
                {
                    cachedVideoFrames.push_back(key);
                }
                float cachedVideoPercentage = 0.F;
                if (thread.state.cacheOptions.videoGB > 0.F)
                {
                    cachedVideoPercentage =
                        (thread.videoCache.getSize() / static_cast<float>(dtk::gigabyte)) /
                        thread.state.cacheOptions.videoGB *
                        100.F;
                }
                std::vector<OTIO_NS::RationalTime> cachedAudioFrames;
                float cachedAudioPercentage = 0.F;
                {
                    size_t cacheSize = 0;
                    std::vector<int64_t> cacheKeys;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        cacheSize = audioMutex.cache.getSize();
                        cacheKeys = audioMutex.cache.getKeys();
                    }
                    for (const auto& key : cacheKeys)
                    {
                        cachedAudioFrames.push_back(OTIO_NS::RationalTime(key, 1.0));
                    }
                    if (thread.state.cacheOptions.audioGB > 0.F)
                    {
                        cachedAudioPercentage =
                            (cacheSize / static_cast<float>(dtk::gigabyte)) /
                            thread.state.cacheOptions.audioGB *
                            100.F;
                    }
                }

                auto cachedVideoRanges = toRanges(cachedVideoFrames);
                auto cachedAudioRanges = toRanges(cachedAudioFrames);
                for (auto& i : cachedAudioRanges)
                {
                    i = OTIO_NS::TimeRange(
                        i.start_time().rescaled_to(timeRange.duration().rate()).floor(),
                        i.duration().rescaled_to(timeRange.duration().rate()).ceil());
                }
                {
                    std::unique_lock<std::mutex> lock(mutex.mutex);
                    mutex.cacheInfo.videoPercentage = cachedVideoPercentage;
                    mutex.cacheInfo.audioPercentage = cachedAudioPercentage;
                    mutex.cacheInfo.video = cachedVideoRanges;
                    mutex.cacheInfo.audio = cachedAudioRanges;
                }
            }
        }

        void Player::Private::playbackReset(const OTIO_NS::RationalTime& time)
        {
            noAudio.playbackTimer = std::chrono::steady_clock::now();
            noAudio.start = time;
        }

        void Player::Private::log(const std::shared_ptr<dtk::Context>& context)
        {
            const std::string id = dtk::Format("tl::timeline::Player {0}").arg(this);

            // Get values.
            OTIO_NS::RationalTime currentTime = time::invalidTime;
            OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
            io::Options ioOptions;
            PlayerCacheInfo cacheInfo;
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                currentTime = mutex.state.currentTime;
                inOutRange = mutex.state.inOutRange;
                ioOptions = mutex.state.ioOptions;
                cacheInfo = mutex.cacheInfo;
            }
            const size_t videoCacheByteCount = thread.videoCache.getSize();
            size_t audioCacheByteCount = 0;
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioCacheByteCount = audioMutex.cache.getSize();
            }

            // Create an array of characters to draw the timeline.
            const size_t lineLength = 80;
            std::string currentTimeDisplay(lineLength, '.');
            double n = (currentTime - timeRange.start_time()).value() / timeRange.duration().value();
            size_t index = dtk::clamp(n, 0.0, 1.0) * (lineLength - 1);
            if (index < currentTimeDisplay.size())
            {
                currentTimeDisplay[index] = 'T';
            }

            // Create an array of characters to draw the cached video frames.
            std::string cachedVideoFramesDisplay(lineLength, '.');
            for (const auto& i : cacheInfo.video)
            {
                n = (i.start_time() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t0 = dtk::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t1 = dtk::clamp(n, 0.0, 1.0) * (lineLength - 1);
                for (size_t j = t0; j <= t1; ++j)
                {
                    if (j < cachedVideoFramesDisplay.size())
                    {
                        cachedVideoFramesDisplay[j] = 'V';
                    }
                }
            }

            // Create an array of characters to draw the cached audio frames.
            std::string cachedAudioFramesDisplay(lineLength, '.');
            for (const auto& i : cacheInfo.audio)
            {
                double n = (i.start_time() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t0 = dtk::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t1 = dtk::clamp(n, 0.0, 1.0) * (lineLength - 1);
                for (size_t j = t0; j <= t1; ++j)
                {
                    if (j < cachedAudioFramesDisplay.size())
                    {
                        cachedAudioFramesDisplay[j] = 'A';
                    }
                }
            }

            std::vector<std::string> ioOptionStrings;
            for (const auto& i : ioOptions)
            {
                ioOptionStrings.push_back(dtk::Format("{0}:{1}").arg(i.first).arg(i.second));
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(id, dtk::Format(
                "\n"
                "    Path: {0}\n"
                "    Current time: {1}\n"
                "    In/out range: {2}\n"
                "    I/O options: {3}\n"
                "    Video cache: {4}/{5}GB\n"
                "    Audio cache: {6}/{7}GB\n"
                "    Read behind: {8}GB\n"
                "    Video requests: {9}\n"
                "    Audio requests: {10}\n"
                "    {11}\n"
                "    {12}\n"
                "    {13}\n"
                "    (T=current time, V=cached video, A=cached audio)").
                arg(timeline->getPath().get()).
                arg(currentTime).
                arg(inOutRange).
                arg(dtk::join(ioOptionStrings, ", ")).
                arg(videoCacheByteCount / static_cast<float>(dtk::gigabyte)).
                arg(cacheOptions->get().videoGB).
                arg(audioCacheByteCount / static_cast<float>(dtk::gigabyte)).
                arg(cacheOptions->get().audioGB).
                arg(cacheOptions->get().readBehind).
                arg(thread.videoDataRequests.size()).
                arg(thread.audioDataRequests.size()).
                arg(currentTimeDisplay).
                arg(cachedVideoFramesDisplay).
                arg(cachedAudioFramesDisplay));
        }

        bool Player::Private::PlaybackState::operator == (const PlaybackState& other) const
        {
            return
                playback == other.playback &&
                currentTime == other.currentTime &&
                inOutRange == other.inOutRange &&
                compare == other.compare &&
                compareTime == other.compareTime &&
                ioOptions == other.ioOptions &&
                videoLayer == other.videoLayer &&
                compareVideoLayers == other.compareVideoLayers &&
                audioOffset == other.audioOffset &&
                cacheOptions == other.cacheOptions;
        }

        bool Player::Private::PlaybackState::operator != (const PlaybackState& other) const
        {
            return !(*this == other);
        }

        bool Player::Private::AudioState::operator == (const AudioState& other) const
        {
            return
                playback == other.playback &&
                speed == other.speed &&
                volume == other.volume &&
                mute == other.mute &&
                channelMute == other.channelMute &&
                muteTimeout == other.muteTimeout &&
                audioOffset == other.audioOffset;
        }

        bool Player::Private::AudioState::operator != (const AudioState& other) const
        {
            return !(*this == other);
        }
    }
}
