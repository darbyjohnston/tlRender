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
                        mutex.playback = Playback::Stop;
                        mutex.clearRequests = true;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.playback = Playback::Stop;
                    }
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Stop);
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.playback = Playback::Stop;
                        mutex.clearRequests = true;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.playback = Playback::Stop;
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
                        mutex.playback = Playback::Forward;
                        mutex.currentTime = out;
                        mutex.clearRequests = true;
                        mutex.cacheDirection = CacheDirection::Forward;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.playback = Playback::Forward;
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
                        mutex.playback = Playback::Reverse;
                        mutex.currentTime = out;
                        mutex.clearRequests = true;
                        mutex.cacheDirection = CacheDirection::Reverse;
                    }
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.playback = Playback::Reverse;
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
            std::vector<std::vector<uint64_t> > ids(1 + thread.compare.size());
            for (const auto& i : thread.videoDataRequests)
            {
                for (size_t j = 0; j < i.second.size() && j < ids.size(); ++j)
                {
                    ids[j].push_back(i.second[j].id);
                }
            }
            for (const auto& i : thread.audioDataRequests)
            {
                ids[0].push_back(i.second.id);
            }
            timeline->cancelRequests(ids[0]);
            for (size_t i = 0; i < thread.compare.size(); ++i)
            {
                thread.compare[i]->cancelRequests(ids[i + 1]);
            }
            thread.videoDataRequests.clear();
            thread.audioDataRequests.clear();
        }

        void Player::Private::clearCache()
        {
            thread.videoDataCache.clear();
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                mutex.cacheInfo = PlayerCacheInfo();
            }
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.audioDataCache.clear();
            }
        }

        void Player::Private::cacheUpdate()
        {
            //std::cout << "current time: " << currentTime->get() << std::endl;

            thread.videoDataCache.setMax(thread.cacheOptions.videoGB * dtk::gigabyte);
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.audioDataCache.setMax(thread.cacheOptions.audioGB * dtk::gigabyte);
            }

            // Fill the video cache.
            if (!ioInfo.video.empty())
            {
                const int64_t duration = thread.inOutRange.duration().value();
                const int64_t readBehind = OTIO_NS::RationalTime(
                    thread.cacheOptions.readBehind, 1.0).rescaled_to(thread.currentTime.rate()).value();
                size_t byteCount = 0;
                for (int64_t frame = 0;
                    frame < duration && thread.videoDataRequests.size() < 16;
                    ++frame)
                {
                    const OTIO_NS::RationalTime t = timeline::loop(
                        OTIO_NS::RationalTime(
                            thread.currentTime.value() - readBehind + frame,
                            thread.currentTime.rate()),
                        thread.inOutRange);
                    byteCount += timeline->getVideoSize(t).future.get();
                    if (byteCount >= thread.videoDataCache.getMax())
                    {
                        break;
                    }
                    const auto i = thread.videoDataRequests.find(t);
                    if (i == thread.videoDataRequests.end() && !thread.videoDataCache.contains(t))
                    {
                        //std::cout << this << " video request: " << t << std::endl;
                        auto& request = thread.videoDataRequests[t];
                        request.clear();
                        io::Options ioOptions2 = thread.ioOptions;
                        ioOptions2["Layer"] = dtk::Format("{0}").arg(thread.videoLayer);
                        request.push_back(timeline->getVideo(t, ioOptions2));
                        for (size_t i = 0; i < thread.compare.size(); ++i)
                        {
                            const OTIO_NS::RationalTime time2 = timeline::getCompareTime(
                                t,
                                timeRange,
                                thread.compare[i]->getTimeRange(),
                                thread.compareTime);
                            ioOptions2["Layer"] = dtk::Format("{0}").
                                arg(i < thread.compareVideoLayers.size() ?
                                    thread.compareVideoLayers[i] :
                                    thread.videoLayer);
                            request.push_back(thread.compare[i]->getVideo(time2, ioOptions2));
                        }
                    }
                }
            }

            // Fill the audio cache.
            if (ioInfo.audio.isValid())
            {
                size_t cacheMax = 0;
                std::vector<int64_t> cacheKeys;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    cacheMax = audioMutex.audioDataCache.getMax();
                    cacheKeys = audioMutex.audioDataCache.getKeys();
                }
                const int64_t duration = thread.inOutRange.duration().rescaled_to(1.0).value();
                size_t byteCount = 0;
                for (int64_t seconds = 0;
                    seconds < duration && thread.audioDataRequests.size() < 16;
                    ++seconds)
                {
                    const int64_t t = timeline::loop(
                        thread.currentTime.rescaled_to(1.0).floor().value() -
                        thread.cacheOptions.readBehind +
                        seconds,
                        thread.inOutRange);
                    byteCount += timeline->getAudioSize(t).future.get();
                    if (byteCount >= cacheMax)
                    {
                        break;
                    }
                    const auto i = thread.audioDataRequests.find(t);
                    if (i == thread.audioDataRequests.end())
                    {
                        const auto j = std::find(cacheKeys.begin(), cacheKeys.end(), t);
                        if (j == cacheKeys.end())
                        {
                            thread.audioDataRequests[t] = timeline->getAudio(t, thread.ioOptions);
                        }
                    }
                }
            }

            // Check for finished video.
            auto videoDataRequestsIt = thread.videoDataRequests.begin();
            while (videoDataRequestsIt != thread.videoDataRequests.end())
            {
                bool ready = true;
                for (auto videoDataRequestIt = videoDataRequestsIt->second.begin();
                    videoDataRequestIt != videoDataRequestsIt->second.end();
                    ++videoDataRequestIt)
                {
                    ready &= videoDataRequestIt->future.valid() &&
                        videoDataRequestIt->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                }
                if (ready)
                {
                    const OTIO_NS::RationalTime time = videoDataRequestsIt->first;
                    std::vector<VideoData> videoDataList;
                    for (auto videoDataRequestIt = videoDataRequestsIt->second.begin();
                        videoDataRequestIt != videoDataRequestsIt->second.end();
                        ++videoDataRequestIt)
                    {
                        auto videoData = videoDataRequestIt->future.get();
                        videoData.time = time;
                        videoDataList.emplace_back(videoData);
                    }
                    thread.videoDataCache.add(time, videoDataList, timeline->getVideoSize(time).future.get());
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
                if (audioDataRequestsIt->second.future.valid() &&
                    audioDataRequestsIt->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto audioData = audioDataRequestsIt->second.future.get();
                    audioData.seconds = audioDataRequestsIt->first;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.audioDataCache.add(
                            audioDataRequestsIt->first,
                            audioData,
                            timeline->getAudioSize(audioDataRequestsIt->first).future.get());
                    }
                    audioDataRequestsIt = thread.audioDataRequests.erase(audioDataRequestsIt);
                }
                else
                {
                    ++audioDataRequestsIt;
                }
            }

            // Update cached frames.
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - thread.cacheTimer;
            if (diff.count() > .5F)
            {
                thread.cacheTimer = now;
                std::vector<OTIO_NS::RationalTime> cachedVideoFrames;
                for (const auto& key : thread.videoDataCache.getKeys())
                {
                    cachedVideoFrames.push_back(key);
                }
                float cachedVideoPercentage = 0.F;
                if (thread.cacheOptions.videoGB > 0.F)
                {
                    cachedVideoPercentage =
                        (thread.videoDataCache.getSize() / static_cast<float>(dtk::gigabyte)) /
                        thread.cacheOptions.videoGB *
                        100.F;
                }
                std::vector<OTIO_NS::RationalTime> cachedAudioFrames;
                float cachedAudioPercentage = 0.F;
                {
                    size_t size = 0;
                    std::vector<int64_t> keys;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        size = audioMutex.audioDataCache.getSize();
                        keys = audioMutex.audioDataCache.getKeys();
                    }
                    for (const auto& key : keys)
                    {
                        cachedAudioFrames.push_back(OTIO_NS::RationalTime(
                            timeRange.start_time().rescaled_to(1.0).value() + key,
                            1.0));
                    }
                    if (thread.cacheOptions.audioGB > 0.F)
                    {
                        cachedAudioPercentage =
                            (size / static_cast<float>(dtk::gigabyte)) /
                            thread.cacheOptions.audioGB *
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
                currentTime = mutex.currentTime;
                inOutRange = mutex.inOutRange;
                ioOptions = mutex.ioOptions;
                cacheInfo = mutex.cacheInfo;
            }
            const size_t videoCacheByteCount = thread.videoDataCache.getSize();
            size_t audioCacheByteCount = 0;
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioCacheByteCount = audioMutex.audioDataCache.getSize();
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
    }
}
