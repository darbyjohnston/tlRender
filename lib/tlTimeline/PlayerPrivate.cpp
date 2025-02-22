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

            // Get the video ranges to be cached.
            const OTIO_NS::RationalTime readAheadDivided(
                thread.cacheOptions.readAhead.value() / static_cast<double>(1 + thread.compare.size()),
                thread.cacheOptions.readAhead.rate());
            const OTIO_NS::RationalTime readAheadRescaled = readAheadDivided.
                rescaled_to(timeRange.duration().rate()).
                floor();
            const OTIO_NS::RationalTime readBehindDivided(
                thread.cacheOptions.readBehind.value() / static_cast<double>(1 + thread.compare.size()),
                thread.cacheOptions.readBehind.rate());
            const OTIO_NS::RationalTime readBehindRescaled = readBehindDivided.
                rescaled_to(timeRange.duration().rate()).
                floor();
            OTIO_NS::TimeRange videoRange = time::invalidTimeRange;
            switch (thread.cacheDirection)
            {
            case CacheDirection::Forward:
                videoRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                    thread.currentTime - readBehindRescaled,
                    thread.currentTime + readAheadRescaled);
                break;
            case CacheDirection::Reverse:
                videoRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                    thread.currentTime - readAheadRescaled,
                    thread.currentTime + readBehindRescaled);
                break;
            default: break;
            }
            //std::cout << "in out range: " << inOutRange << std::endl;
            //std::cout << "video range: " << videoRange << std::endl;
            auto videoRanges = timeline::loopCache(
                videoRange,
                thread.inOutRange,
                thread.cacheDirection);
            videoRanges.insert(
                videoRanges.begin(),
                OTIO_NS::TimeRange(
                    thread.currentTime,
                    OTIO_NS::RationalTime(1.0, thread.currentTime.rate())));
            //for (const auto& i : videoRanges)
            //{
            //    std::cout << "video ranges: " << i << std::endl;
            //}

            // Get the audio ranges to be cached.
            OTIO_NS::RationalTime audioOffsetTime = OTIO_NS::RationalTime(thread.audioOffset, 1.0).
                rescaled_to(timeRange.duration().rate());
            //std::cout << "audio offset: " << audioOffsetTime << std::endl;
            OTIO_NS::TimeRange audioRange = time::invalidTimeRange;
            switch (thread.cacheDirection)
            {
            case CacheDirection::Forward:
                audioRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                    thread.currentTime - readBehindRescaled - audioOffsetTime,
                    thread.currentTime + readAheadRescaled - audioOffsetTime);
                break;
            case CacheDirection::Reverse:
                audioRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                    thread.currentTime - readAheadRescaled - audioOffsetTime,
                    thread.currentTime + readBehindRescaled - audioOffsetTime);
                break;
            default: break;
            }
            //std::cout << "audio range: " << audioRange << std::endl;
            const OTIO_NS::TimeRange inOutAudioRange = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                thread.inOutRange.start_time() - audioOffsetTime,
                thread.inOutRange.end_time_inclusive() - audioOffsetTime);
            //std::cout << "in out audio range: " << inOutAudioRange << std::endl;
            const auto audioRanges = timeline::loopCache(
                audioRange,
                inOutAudioRange,
                thread.cacheDirection);

            // Remove old video from the cache.
            auto videoCacheIt = thread.videoDataCache.begin();
            while (videoCacheIt != thread.videoDataCache.end())
            {
                const OTIO_NS::RationalTime t = videoCacheIt->first;
                const auto j = std::find_if(
                    videoRanges.begin(),
                    videoRanges.end(),
                    [t](const OTIO_NS::TimeRange& value)
                    {
                        return value.contains(t);
                    });
                if (j == videoRanges.end())
                {
                    videoCacheIt = thread.videoDataCache.erase(videoCacheIt);
                }
                else
                {
                    ++videoCacheIt;
                }
            }

            // Remove old audio from the cache.
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                auto audioCacheIt = audioMutex.audioDataCache.begin();
                while (audioCacheIt != audioMutex.audioDataCache.end())
                {
                    const OTIO_NS::TimeRange cacheRange(
                        OTIO_NS::RationalTime(
                            timeRange.start_time().rescaled_to(1.0).value() +
                            audioCacheIt->first,
                            1.0),
                        OTIO_NS::RationalTime(1.0, 1.0));
                    const auto j = std::find_if(
                        audioRanges.begin(),
                        audioRanges.end(),
                        [cacheRange](const OTIO_NS::TimeRange& value)
                        {
                            return cacheRange.intersects(value);
                        });
                    if (j == audioRanges.end())
                    {
                        audioCacheIt = audioMutex.audioDataCache.erase(audioCacheIt);
                    }
                    else
                    {
                        ++audioCacheIt;
                    }
                }
            }

            // Get uncached video.
            if (!ioInfo.video.empty())
            {
                for (const auto& range : videoRanges)
                {
                    switch (thread.cacheDirection)
                    {
                    case CacheDirection::Forward:
                    {
                        const OTIO_NS::RationalTime start = range.start_time();
                        const OTIO_NS::RationalTime end = range.end_time_inclusive();
                        const OTIO_NS::RationalTime inc = OTIO_NS::RationalTime(1.0, range.duration().rate());
                        for (OTIO_NS::RationalTime time = start; time <= end; time += inc)
                        {
                            const auto i = thread.videoDataCache.find(time);
                            if (i == thread.videoDataCache.end())
                            {
                                const auto j = thread.videoDataRequests.find(time);
                                if (j == thread.videoDataRequests.end())
                                {
                                    //std::cout << this << " video request: " << time << std::endl;
                                    auto& request = thread.videoDataRequests[time];
                                    request.clear();
                                    io::Options ioOptions2 = thread.ioOptions;
                                    ioOptions2["Layer"] = dtk::Format("{0}").arg(thread.videoLayer);
                                    request.push_back(timeline->getVideo(time, ioOptions2));
                                    for (size_t i = 0; i < thread.compare.size(); ++i)
                                    {
                                        const OTIO_NS::RationalTime time2 = timeline::getCompareTime(
                                            time,
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
                        break;
                    }
                    case CacheDirection::Reverse:
                    {
                        const auto start = range.end_time_inclusive();
                        const auto end = range.start_time();
                        const auto inc = OTIO_NS::RationalTime(1.0, range.duration().rate());
                        for (auto time = start; time >= end; time -= inc)
                        {
                            const auto i = thread.videoDataCache.find(time);
                            if (i == thread.videoDataCache.end())
                            {
                                const auto j = thread.videoDataRequests.find(time);
                                if (j == thread.videoDataRequests.end())
                                {
                                    //std::cout << this << " video request: " << time << std::endl;
                                    auto& request = thread.videoDataRequests[time];
                                    request.clear();
                                    io::Options ioOptions2 = thread.ioOptions;
                                    ioOptions2["Layer"] = dtk::Format("{0}").arg(thread.videoLayer);
                                    request.push_back(timeline->getVideo(time, ioOptions2));
                                    for (size_t i = 0; i < thread.compare.size(); ++i)
                                    {
                                        const OTIO_NS::RationalTime time2 = timeline::getCompareTime(
                                            time,
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
                        break;
                    }
                    default: break;
                    }
                }
            }

            // Get uncached audio.
            if (ioInfo.audio.isValid())
            {
                std::set<int64_t> seconds;
                for (const auto& range : audioRanges)
                {
                    const int64_t start = range.start_time().rescaled_to(1.0).value() -
                        timeRange.start_time().rescaled_to(1.0).value();
                    const int64_t end = start + range.duration().rescaled_to(1.0).value();
                    for (int64_t time = start; time <= end; ++time)
                    {
                        seconds.insert(time);
                    }
                }
                std::map<int64_t, double> requests;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    for (int64_t s : seconds)
                    {
                        const auto i = audioMutex.audioDataCache.find(s);
                        if (i == audioMutex.audioDataCache.end())
                        {
                            const auto j = thread.audioDataRequests.find(s);
                            if (j == thread.audioDataRequests.end())
                            {
                                requests[s] = timeRange.start_time().rescaled_to(1.0).value() + s;
                            }
                        }
                    }
                }
                switch (thread.cacheDirection)
                {
                case CacheDirection::Forward:
                    for (auto i = requests.begin(); i != requests.end(); ++i)
                    {
                        thread.audioDataRequests[i->first] = timeline->getAudio(i->second, thread.ioOptions);
                    }
                    break;
                case CacheDirection::Reverse:
                    for (auto i = requests.rbegin(); i != requests.rend(); ++i)
                    {
                        thread.audioDataRequests[i->first] = timeline->getAudio(i->second, thread.ioOptions);
                    }
                    break;
                default: break;
                }
                /*if (!requests.empty())
                {
                    std::cout << "audio range: " << audioRange << std::endl;
                    std::cout << "audio request:";
                    for (auto i : requests)
                    {
                        std::cout << " " << i;
                    }
                    std::cout << std::endl;
                }*/
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
                    auto& videoDataCache = thread.videoDataCache[time];
                    videoDataCache.clear();
                    for (auto videoDataRequestIt = videoDataRequestsIt->second.begin();
                        videoDataRequestIt != videoDataRequestsIt->second.end();
                        ++videoDataRequestIt)
                    {
                        auto videoData = videoDataRequestIt->future.get();
                        videoData.time = time;
                        videoDataCache.push_back(videoData);
                    }
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
                        audioMutex.audioDataCache[audioDataRequestsIt->first] = audioData;
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
                for (const auto& i : thread.videoDataCache)
                {
                    cachedVideoFrames.push_back(i.first);
                }
                const float cachedVideoPercentage = cachedVideoFrames.size() /
                    static_cast<float>(readAheadDivided.rescaled_to(timeRange.duration().rate()).value() +
                        readBehindDivided.rescaled_to(timeRange.duration().rate()).value()) *
                    100.F;
                std::vector<OTIO_NS::RationalTime> cachedAudioFrames;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    for (const auto& i : audioMutex.audioDataCache)
                    {
                        cachedAudioFrames.push_back(OTIO_NS::RationalTime(
                            timeRange.start_time().rescaled_to(1.0).value() + i.first,
                            1.0));
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
                float cachedAudioPercentage = 0.F;
                {
                    std::unique_lock<std::mutex> lock(mutex.mutex);
                    mutex.cacheInfo.videoPercentage = cachedVideoPercentage;
                    mutex.cacheInfo.videoFrames = cachedVideoRanges;
                    mutex.cacheInfo.audioFrames = cachedAudioRanges;
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

            // Get mutex protected values.
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
            size_t audioDataCacheSize = 0;
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioDataCacheSize = audioMutex.audioDataCache.size();
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
            for (const auto& i : cacheInfo.videoFrames)
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
            for (const auto& i : cacheInfo.audioFrames)
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
                "    Cache: {4} read ahead, {5} read behind\n"
                "    Video: {6} requests, {7} cached\n"
                "    Audio: {8} requests, {9} cached\n"
                "    {10}\n"
                "    {11}\n"
                "    {12}\n"
                "    (T=current time, V=cached video, A=cached audio)").
                arg(timeline->getPath().get()).
                arg(currentTime).
                arg(inOutRange).
                arg(dtk::join(ioOptionStrings, ", ")).
                arg(cacheOptions->get().readAhead).
                arg(cacheOptions->get().readBehind).
                arg(thread.videoDataRequests.size()).
                arg(thread.videoDataCache.size()).
                arg(thread.audioDataRequests.size()).
                arg(audioDataCacheSize).
                arg(currentTimeDisplay).
                arg(cachedVideoFramesDisplay).
                arg(cachedAudioFramesDisplay));
        }
    }
}
