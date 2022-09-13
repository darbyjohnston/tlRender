// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        otime::RationalTime TimelinePlayer::Private::loopPlayback(const otime::RationalTime& time)
        {
            otime::RationalTime out = time;

            const auto& range = inOutRange->get();
            switch (loop->get())
            {
            case Loop::Loop:
            {
                bool looped = false;
                out = timeline::loop(out, range, &looped);
                if (looped)
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playbackStartTime = out;
                        mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                    }
                    resetAudioTime();
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
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playback = Playback::Stop;
                        mutexData.clearRequests = true;
                    }
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Stop);
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playback = Playback::Stop;
                        mutexData.clearRequests = true;
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
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playback = Playback::Forward;
                        mutexData.playbackStartTime = out;
                        mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                        mutexData.currentTime = currentTime->get();
                        mutexData.clearRequests = true;
                        mutexData.cacheDirection = CacheDirection::Forward;
                    }
                    resetAudioTime();
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Reverse);
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playback = Playback::Reverse;
                        mutexData.playbackStartTime = out;
                        mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                        mutexData.currentTime = currentTime->get();
                        mutexData.clearRequests = true;
                        mutexData.cacheDirection = CacheDirection::Reverse;
                    }
                    resetAudioTime();
                }
                break;
            }
            default: break;
            }

            return out;
        }

        void TimelinePlayer::Private::cacheUpdate(
            const otime::RationalTime& currentTime,
            const otime::TimeRange& inOutRange,
            uint16_t videoLayer,
            double audioOffset,
            CacheDirection cacheDirection,
            const otime::RationalTime& cacheReadAhead,
            const otime::RationalTime& cacheReadBehind)
        {
            // Get the video ranges to be cached.
            const otime::RationalTime& globalStartTime = timeline->getGlobalStartTime();
            const otime::RationalTime& duration = timeline->getDuration();
            const otime::RationalTime cacheReadAheadRescaled =
                time::floor(cacheReadAhead.rescaled_to(duration.rate()));
            const otime::RationalTime cacheReadBehindRescaled =
                time::floor(cacheReadBehind.rescaled_to(duration.rate()));
            otime::TimeRange videoRange = time::invalidTimeRange;
            switch (cacheDirection)
            {
            case CacheDirection::Forward:
                videoRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - cacheReadBehindRescaled,
                    currentTime + cacheReadAheadRescaled);
                break;
            case CacheDirection::Reverse:
                videoRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - cacheReadAheadRescaled,
                    currentTime + cacheReadBehindRescaled);
                break;
            default: break;
            }
            //std::cout << "in out range: " << inOutRange << std::endl;
            //std::cout << "video range: " << videoRange << std::endl;
            const auto videoRanges = timeline::loop(videoRange, inOutRange);
            //for (const auto& i : videoRanges)
            //{
            //    std::cout << "video ranges: " << i << std::endl;
            //}

            // Get the audio ranges to be cached.
            const otime::RationalTime audioOffsetTime = otime::RationalTime(audioOffset, 1.0).
                rescaled_to(duration.rate());
            //std::cout << "audio offset: " << audioOffsetTime << std::endl;
            const otime::RationalTime audioOffsetAhead = time::round(
                audioOffsetTime.value() < 0.0 ? -audioOffsetTime : otime::RationalTime(0.0, duration.rate()));
            const otime::RationalTime audioOffsetBehind = time::round(
                audioOffsetTime.value() > 0.0 ? audioOffsetTime : otime::RationalTime(0.0, duration.rate()));
            //std::cout << "audio offset ahead: " << audioOffsetAhead << std::endl;
            //std::cout << "audio offset behind: " << audioOffsetBehind << std::endl;
            otime::TimeRange audioRange = time::invalidTimeRange;
            switch (cacheDirection)
            {
            case CacheDirection::Forward:
                audioRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - cacheReadBehindRescaled - audioOffsetBehind,
                    currentTime + cacheReadAheadRescaled + audioOffsetAhead);
                break;
            case CacheDirection::Reverse:
                audioRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - cacheReadAheadRescaled - audioOffsetAhead,
                    currentTime + cacheReadBehindRescaled + audioOffsetBehind);
                break;
            default: break;
            }
            //std::cout << "audio range: " << audioRange << std::endl;
            const otime::TimeRange totalRange(globalStartTime, duration);
            //std::cout << "total range: " << totalRange << std::endl;
            const otime::TimeRange inOutAudioRange = otime::TimeRange::range_from_start_end_time_inclusive(
                inOutRange.start_time() - audioOffsetBehind,
                inOutRange.end_time_inclusive() + audioOffsetAhead).
                    clamped(totalRange);
            //std::cout << "in out audio range: " << inOutAudioRange << std::endl;
            const auto audioRanges = timeline::loop(audioRange, inOutAudioRange);
            std::vector<otime::TimeRange> audioCacheRanges;
            for (const auto& i : audioRanges)
            {
                const otime::TimeRange range = otime::TimeRange::range_from_start_end_time_inclusive(
                    time::floor(i.start_time().rescaled_to(1.0)),
                    time::ceil(i.end_time_inclusive().rescaled_to(1.0)));
                //std::cout << "audio ranges: " << range.start_time() <<  " " <<
                //    range.end_time_inclusive() << std::endl;
                audioCacheRanges.push_back(range);
            }
            //std::cout << std::endl;
            timeline->setActiveRanges(audioRanges);

            // Remove old video from the cache.
            auto videoDataCacheIt = threadData.videoDataCache.begin();
            while (videoDataCacheIt != threadData.videoDataCache.end())
            {
                bool old = true;
                for (const auto& i : videoRanges)
                {
                    if (i.contains(videoDataCacheIt->second.time))
                    {
                        old = false;
                        break;
                    }
                }
                if (old)
                {
                    videoDataCacheIt = threadData.videoDataCache.erase(videoDataCacheIt);
                    continue;
                }
                ++videoDataCacheIt;
            }

            // Remove old audio from the cache.
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                auto audioDataCacheIt = audioMutexData.audioDataCache.begin();
                while (audioDataCacheIt != audioMutexData.audioDataCache.end())
                {
                    bool old = true;
                    for (const auto& i : audioRanges)
                    {
                        if (i.intersects(otime::TimeRange(
                            otime::RationalTime(audioDataCacheIt->second.seconds, 1.0),
                            otime::RationalTime(1.0, 1.0))))
                        {
                            old = false;
                            break;
                        }
                    }
                    if (old)
                    {
                        //std::cout << "audio remove: " << audioDataCacheIt->second.seconds << std::endl;
                        audioDataCacheIt = audioMutexData.audioDataCache.erase(audioDataCacheIt);
                        continue;
                    }
                    ++audioDataCacheIt;
                }
            }

            // Get uncached video.
            if (!ioInfo.video.empty())
            {
                for (const auto& i : videoRanges)
                {
                    for (otime::RationalTime time = i.start_time();
                        time < i.end_time_exclusive();
                        time += otime::RationalTime(1.0, duration.rate()))
                    {
                        const auto j = threadData.videoDataCache.find(time);
                        if (j == threadData.videoDataCache.end())
                        {
                            const auto k = threadData.videoDataRequests.find(time);
                            if (k == threadData.videoDataRequests.end())
                            {
                                //std::cout << this << " video request: " << time << std::endl;
                                threadData.videoDataRequests[time] = timeline->getVideo(time, videoLayer);
                            }
                        }
                    }
                }
            }

            // Get uncached audio.
            if (ioInfo.audio.isValid())
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                for (const auto& i : audioCacheRanges)
                {
                    for (auto j = i.start_time(); j < i.end_time_inclusive(); j += otime::RationalTime(1.0, 1.0))
                    {
                        const int64_t time = j.value();
                        const auto k = audioMutexData.audioDataCache.find(time);
                        if (k == audioMutexData.audioDataCache.end())
                        {
                            const auto l = threadData.audioDataRequests.find(time);
                            if (l == threadData.audioDataRequests.end())
                            {
                                //std::cout << this << " audio request: " << time << std::endl;
                                threadData.audioDataRequests[time] = timeline->getAudio(time);
                            }
                        }
                    }
                }
            }

            // Check for finished video.
            auto videoDataRequestsIt = threadData.videoDataRequests.begin();
            while (videoDataRequestsIt != threadData.videoDataRequests.end())
            {
                if (videoDataRequestsIt->second.valid() &&
                    videoDataRequestsIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto data = videoDataRequestsIt->second.get();
                    data.time = videoDataRequestsIt->first;
                    threadData.videoDataCache[data.time] = data;
                    videoDataRequestsIt = threadData.videoDataRequests.erase(videoDataRequestsIt);
                    continue;
                }
                ++videoDataRequestsIt;
            }

            // Check for finished audio.
            auto audioDataRequestsIt = threadData.audioDataRequests.begin();
            while (audioDataRequestsIt != threadData.audioDataRequests.end())
            {
                if (audioDataRequestsIt->second.valid() &&
                    audioDataRequestsIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto data = audioDataRequestsIt->second.get();
                    data.seconds = audioDataRequestsIt->first;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex);
                        audioMutexData.audioDataCache[data.seconds] = data;
                    }
                    audioDataRequestsIt = threadData.audioDataRequests.erase(audioDataRequestsIt);
                    continue;
                }
                ++audioDataRequestsIt;
            }

            // Update cached frames.
            std::vector<otime::RationalTime> cachedVideoFrames;
            for (const auto& i : threadData.videoDataCache)
            {
                cachedVideoFrames.push_back(i.second.time);
            }
            std::vector<otime::RationalTime> cachedAudioFrames;
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                for (const auto& i : audioMutexData.audioDataCache)
                {
                    cachedAudioFrames.push_back(otime::RationalTime(i.second.seconds, 1.0));
                }
            }
            auto cachedVideoRanges = toRanges(cachedVideoFrames);
            auto cachedAudioRanges = toRanges(cachedAudioFrames);
            for (auto& i : cachedAudioRanges)
            {
                i = otime::TimeRange(
                    time::floor(i.start_time().rescaled_to(duration.rate())),
                    time::ceil(i.duration().rescaled_to(duration.rate())));
            }
            {
                std::unique_lock<std::mutex> lock(mutex);
                mutexData.cachedVideoFrames = cachedVideoRanges;
                mutexData.cachedAudioFrames = cachedAudioRanges;
            }
        }

        void TimelinePlayer::Private::resetAudioTime()
        {
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                audioMutexData.rtAudioCurrentFrame = 0;
            }
            if (threadData.rtAudio &&
                threadData.rtAudio->isStreamRunning())
            {
                try
                {
                    threadData.rtAudio->setStreamTime(0.0);
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }
            }
        }

        int TimelinePlayer::Private::rtAudioCallback(
            void* outputBuffer,
            void* inputBuffer,
            unsigned int nFrames,
            double streamTime,
            RtAudioStreamStatus status,
            void* userData)
        {
            auto p = reinterpret_cast<TimelinePlayer::Private*>(userData);
            
            // Get mutex protected values.
            Playback playback = Playback::Stop;
            double playbackStartTimeInSeconds = 0.0;
            bool externalTime = false;
            {
                std::unique_lock<std::mutex> lock(p->mutex);
                playback = p->mutexData.playback;
                playbackStartTimeInSeconds =
                    p->mutexData.playbackStartTime.rescaled_to(1.0).value() -
                    p->mutexData.audioOffset;
                externalTime = p->mutexData.externalTime;
            }
            double speed = 0.F;
            float volume = 1.F;
            bool mute = false;
            std::chrono::steady_clock::time_point muteTimeout;
            size_t rtAudioCurrentFrame = 0;
            {
                std::unique_lock<std::mutex> lock(p->audioMutex);
                speed = p->audioMutexData.speed;
                volume = p->audioMutexData.volume;
                mute = p->audioMutexData.mute;
                muteTimeout = p->audioMutexData.muteTimeout;
                rtAudioCurrentFrame = p->audioMutexData.rtAudioCurrentFrame;
            }

            // Audio information constants.
            const uint8_t channelCount = p->ioInfo.audio.channelCount;
            const audio::DataType dataType = p->ioInfo.audio.dataType;
            const size_t byteCount = p->ioInfo.audio.getByteCount();

            // Zero output audio data.
            std::memset(outputBuffer, 0, nFrames * byteCount);

            switch (playback)
            {
            case Playback::Forward:
            {
                // Time in seconds for indexing into the audio cache.
                int64_t seconds = playbackStartTimeInSeconds +
                    rtAudioCurrentFrame / static_cast<double>(p->ioInfo.audio.sampleRate);

                // Offset into the audio data.
                int64_t offset = playbackStartTimeInSeconds * p->ioInfo.audio.sampleRate +
                    rtAudioCurrentFrame -
                    seconds * p->ioInfo.audio.sampleRate;

                const auto now = std::chrono::steady_clock::now();

                // Copy audio data to RtAudio.
                if (speed == p->timeline->getDuration().rate() &&
                    !externalTime &&
                    !mute &&
                    now >= muteTimeout &&
                    offset >= 0)
                {
                    size_t sampleCount = nFrames;
                    uint8_t* outputBufferP = reinterpret_cast<uint8_t*>(outputBuffer);
                    size_t count = 0;
                    while (sampleCount > 0)
                    {
                        // Get audio data from the cache.
                        std::vector<std::shared_ptr<audio::Audio> > audioData;
                        {
                            std::unique_lock<std::mutex> lock(p->audioMutex);
                            const auto j = p->audioMutexData.audioDataCache.find(seconds);
                            if (j != p->audioMutexData.audioDataCache.end())
                            {
                                for (const auto& layer : j->second.layers)
                                {
                                    audioData.push_back(layer.audio);
                                }
                            }
                        }

                        size_t size = sampleCount;
                        if (!audioData.empty() && audioData[0])
                        {
                            // Get pointers to the audio data. Only audio data
                            // that has the same information (channels, data
                            // type, sample rate) is used.
                            std::vector<const uint8_t*> audioDataP;
                            for (size_t j = 0; j < audioData.size(); ++j)
                            {
                                if (audioData[j] && audioData[j]->getInfo() == p->ioInfo.audio)
                                {
                                    audioDataP.push_back(audioData[j]->getData() + offset * byteCount);
                                }
                            }

                            size = std::min(size, static_cast<size_t>(audioData[0]->getSampleCount() - offset));
                            
                            //std::cout << count <<
                            //    " samples: " << sampleCount <<
                            //    " seconds: " << seconds <<
                            //    " frame: " << rtAudioCurrentFrame <<
                            //    " offset: " << offset <<
                            //    " size: " << size << std::endl;
                            //std::memcpy(outputBufferP, audioData[0]->getData() + offset * byteCount, size * byteCount);
                            
                            audio::mix(
                                audioDataP.data(),
                                audioDataP.size(),
                                outputBufferP,
                                volume,
                                size,
                                channelCount,
                                dataType);
                        }

                        offset = 0;
                        sampleCount -= size;
                        ++seconds;
                        outputBufferP += size * byteCount;
                        ++count;
                    }
                }

                // Update the audio frame.
                {
                    std::unique_lock<std::mutex> lock(p->audioMutex);
                    p->audioMutexData.rtAudioCurrentFrame += nFrames;
                }

                break;
            }
            case Playback::Reverse:
                // Update the audio frame.
                {
                    std::unique_lock<std::mutex> lock(p->audioMutex);
                    p->audioMutexData.rtAudioCurrentFrame += nFrames;
                }
                break;
            default: break;
            }

            return 0;
        }

        void TimelinePlayer::Private::rtAudioErrorCallback(
            RtAudioError::Type type,
            const std::string& errorText)
        {}

        void TimelinePlayer::Private::log(const std::shared_ptr<system::Context>& context)
        {
            const std::string id = string::Format("tl::timeline::TimelinePlayer {0}").arg(this);

            // Get mutex protected values.
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            uint16_t videoLayer = 0;
            std::vector<otime::TimeRange> cachedVideoFrames;
            std::vector<otime::TimeRange> cachedAudioFrames;
            otime::RationalTime cacheReadAhead = time::invalidTime;
            otime::RationalTime cacheReadBehind = time::invalidTime;
            {
                std::unique_lock<std::mutex> lock(mutex);
                currentTime = mutexData.currentTime;
                inOutRange = mutexData.inOutRange;
                videoLayer = mutexData.videoLayer;
                cachedVideoFrames = mutexData.cachedVideoFrames;
                cachedAudioFrames = mutexData.cachedAudioFrames;
                cacheReadAhead = mutexData.cacheReadAhead;
                cacheReadBehind = mutexData.cacheReadBehind;
            }
            size_t audioDataCacheSize = 0;
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                audioDataCacheSize = audioMutexData.audioDataCache.size();
            }

            // Create an array of characters to draw the timeline.
            const size_t lineLength = 80;
            std::string currentTimeDisplay(lineLength, '.');
            double n = (currentTime - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
            currentTimeDisplay[math::clamp(n, 0.0, 1.0) * (lineLength - 1)] = 'T';

            // Create an array of characters to draw the cached video frames.
            std::string cachedVideoFramesDisplay(lineLength, '.');
            for (const auto& i : cachedVideoFrames)
            {
                double n = (i.start_time() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t0 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t1 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                for (size_t j = t0; j <= t1; ++j)
                {
                    cachedVideoFramesDisplay[j] = 'V';
                }
            }

            // Create an array of characters to draw the cached audio frames.
            std::string cachedAudioFramesDisplay(lineLength, '.');
            for (const auto& i : cachedAudioFrames)
            {
                double n = (i.start_time() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t0 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t1 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                for (size_t j = t0; j <= t1; ++j)
                {
                    cachedAudioFramesDisplay[j] = 'A';
                }
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(id, string::Format(
                "\n"
                "    Path: {0}\n"
                "    Current time: {1}\n"
                "    In/out range: {2}\n"
                "    Video layer: {3}\n"
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
                arg(videoLayer).
                arg(cacheReadAhead).
                arg(cacheReadBehind).
                arg(threadData.videoDataRequests.size()).
                arg(threadData.videoDataCache.size()).
                arg(threadData.audioDataRequests.size()).
                arg(audioDataCacheSize).
                arg(currentTimeDisplay).
                arg(cachedVideoFramesDisplay).
                arg(cachedAudioFramesDisplay));
        }
    }
}
