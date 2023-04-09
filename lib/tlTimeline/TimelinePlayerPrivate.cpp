// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.playbackStartTime = out;
                        mutex.playbackStartTimer = std::chrono::steady_clock::now();
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
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.playback = Playback::Stop;
                        mutex.clearRequests = true;
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
                        mutex.playbackStartTime = out;
                        mutex.playbackStartTimer = std::chrono::steady_clock::now();
                        mutex.currentTime = currentTime->get();
                        mutex.clearRequests = true;
                        mutex.cacheDirection = CacheDirection::Forward;
                    }
                    resetAudioTime();
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Reverse);
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        mutex.playback = Playback::Reverse;
                        mutex.playbackStartTime = out;
                        mutex.playbackStartTimer = std::chrono::steady_clock::now();
                        mutex.currentTime = currentTime->get();
                        mutex.clearRequests = true;
                        mutex.cacheDirection = CacheDirection::Reverse;
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
            const PlayerCacheOptions& cacheOptions)
        {
            // Get the video ranges to be cached.
            const otime::TimeRange& timeRange = timeline->getTimeRange();
            const otime::RationalTime readAheadRescaled =
                time::floor(cacheOptions.readAhead.rescaled_to(timeRange.duration().rate()));
            const otime::RationalTime readBehindRescaled =
                time::floor(cacheOptions.readBehind.rescaled_to(timeRange.duration().rate()));
            
            otime::TimeRange videoRange = time::invalidTimeRange;
            switch (cacheDirection)
            {
            case CacheDirection::Forward:
                videoRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - readBehindRescaled,
                    currentTime + readAheadRescaled);
                break;
            case CacheDirection::Reverse:
                videoRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - readAheadRescaled,
                    currentTime + readBehindRescaled);
                break;
            default: break;
            }

            const auto videoRanges = timeline::loop(videoRange, inOutRange);

            //for (const auto& i : videoRanges)
            //{
            //    std::cout << "video ranges: " << i << std::endl;
            //}

            // Get the audio ranges to be cached.
            const otime::RationalTime audioOffsetTime = otime::RationalTime(audioOffset, 1.0).
                rescaled_to(timeRange.duration().rate());
            //std::cout << "audio offset: " << audioOffsetTime << std::endl;
            const otime::RationalTime audioOffsetAhead = time::round(
                audioOffsetTime.value() < 0.0 ? -audioOffsetTime : otime::RationalTime(0.0, timeRange.duration().rate()));
            const otime::RationalTime audioOffsetBehind = time::round(
                audioOffsetTime.value() > 0.0 ? audioOffsetTime : otime::RationalTime(0.0, timeRange.duration().rate()));
            //std::cout << "audio offset ahead: " << audioOffsetAhead << std::endl;
            //std::cout << "audio offset behind: " << audioOffsetBehind << std::endl;
            otime::TimeRange audioRange = time::invalidTimeRange;
            switch (cacheDirection)
            {
            case CacheDirection::Forward:
                audioRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - readBehindRescaled - audioOffsetBehind,
                    currentTime + readAheadRescaled + audioOffsetAhead);
                break;
            case CacheDirection::Reverse:
                audioRange = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - readAheadRescaled - audioOffsetAhead,
                    currentTime + readBehindRescaled + audioOffsetBehind);
                break;
            default: break;
            }
            //std::cout << "audio range: " << audioRange << std::endl;
            const otime::TimeRange inOutAudioRange = otime::TimeRange::range_from_start_end_time_inclusive(
                inOutRange.start_time() - audioOffsetBehind,
                inOutRange.end_time_inclusive() + audioOffsetAhead).
                    clamped(timeRange);
            //std::cout << "in out audio range: " << inOutAudioRange << std::endl;
            const auto audioRanges = timeline::loop(audioRange, inOutAudioRange);

            // Remove old video from the cache.
            auto videoDataCacheIt = thread.videoDataCache.begin();
            while (videoDataCacheIt != thread.videoDataCache.end())
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
                    videoDataCacheIt = thread.videoDataCache.erase(videoDataCacheIt);
                    continue;
                }
                ++videoDataCacheIt;
            }

            // Remove old audio from the cache.
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                auto audioDataCacheIt = audioMutex.audioDataCache.begin();
                while (audioDataCacheIt != audioMutex.audioDataCache.end())
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
                        audioDataCacheIt = audioMutex.audioDataCache.erase(audioDataCacheIt);
                        continue;
                    }
                    ++audioDataCacheIt;
                }
            }

            // Get uncached video.
            if (!ioInfo.video.empty())
            {
                for (const auto& range : videoRanges)
                {
                    const auto start = range.start_time();
                    const auto end = range.end_time_exclusive();
                    const auto inc = otime::RationalTime(1.0, range.duration().rate());
                    for (auto time = start; time < end; time += inc)
                    {
                        const auto i = thread.videoDataCache.find(time);
                        if (i == thread.videoDataCache.end())
                        {
                            const auto j = thread.videoDataRequests.find(time);
                            if (j == thread.videoDataRequests.end())
                            {
                                //std::cout << this << " video request: " << time << std::endl;
                                thread.videoDataRequests[time] = timeline->getVideo(time, videoLayer);
                            }
                        }
                    }
                }
            }

            // Get uncached audio.
            if (ioInfo.audio.isValid())
            {
                std::set<int64_t> seconds;
                for (const auto& range : audioRanges)
                {
                    const auto start = range.start_time();
                    const auto end = range.end_time_exclusive();
                    const auto inc = otime::RationalTime(1.0, range.duration().rate());
                    for (auto time = start; time < end; time += inc)
                    {
                        seconds.insert(time.rescaled_to(1.0).value());
                    }
                }
                std::vector<int64_t> requests;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    for (const auto& s : seconds)
                    {
                        const auto i = audioMutex.audioDataCache.find(s);
                        if (i == audioMutex.audioDataCache.end())
                        {
                            const auto j = thread.audioDataRequests.find(s);
                            if (j == thread.audioDataRequests.end())
                            {
                                requests.push_back(s);
                            }
                        }
                    }
                }
                for (auto i : requests)
                {
                    thread.audioDataRequests[i] = timeline->getAudio(i);
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
                if (videoDataRequestsIt->second.valid() &&
                    videoDataRequestsIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto data = videoDataRequestsIt->second.get();
                    data.time = videoDataRequestsIt->first;
                    thread.videoDataCache[data.time] = data;
                    videoDataRequestsIt = thread.videoDataRequests.erase(videoDataRequestsIt);
                    continue;
                }
                ++videoDataRequestsIt;
            }

            // Check for finished audio.
            auto audioDataRequestsIt = thread.audioDataRequests.begin();
            while (audioDataRequestsIt != thread.audioDataRequests.end())
            {
                if (audioDataRequestsIt->second.valid() &&
                    audioDataRequestsIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto audioData = audioDataRequestsIt->second.get();
                    audioData.seconds = audioDataRequestsIt->first;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.audioDataCache[audioData.seconds] = audioData;
                    }
                    audioDataRequestsIt = thread.audioDataRequests.erase(audioDataRequestsIt);
                    continue;
                }
                ++audioDataRequestsIt;
            }

            // Update cached frames.
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - thread.cacheTimer;
            if (diff.count() > .5F)
            {
                thread.cacheTimer = now;
                std::vector<otime::RationalTime> cachedVideoFrames;
                for (const auto& i : thread.videoDataCache)
                {
                    cachedVideoFrames.push_back(i.second.time);
                }
                const float cachedVideoPercentage = cachedVideoFrames.size() /
                    static_cast<float>(cacheOptions.readAhead.rescaled_to(timeRange.duration().rate()).value() +
                        cacheOptions.readBehind.rescaled_to(timeRange.duration().rate()).value()) *
                    100.F;
                std::vector<otime::RationalTime> cachedAudioFrames;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    for (const auto& i : audioMutex.audioDataCache)
                    {
                        cachedAudioFrames.push_back(otime::RationalTime(i.second.seconds, 1.0));
                    }
                }
                auto cachedVideoRanges = toRanges(cachedVideoFrames);
                auto cachedAudioRanges = toRanges(cachedAudioFrames);
                for (auto& i : cachedAudioRanges)
                {
                    i = otime::TimeRange(
                        time::floor(i.start_time().rescaled_to(timeRange.duration().rate())),
                        time::ceil(i.duration().rescaled_to(timeRange.duration().rate())));
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

        void TimelinePlayer::Private::resetAudioTime()
        {
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.reset = true;
            }
#if defined(TLRENDER_AUDIO)
            if (thread.rtAudio &&
                thread.rtAudio->isStreamRunning())
            {
                try
                {
                    thread.rtAudio->setStreamTime(0.0);
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }
            }
#endif // TLRENDER_AUDIO
        }

#if defined(TLRENDER_AUDIO)
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
            int64_t playbackStartFrame = 0;
            bool externalTime = false;
            {
                std::unique_lock<std::mutex> lock(p->mutex.mutex);
                playback = p->mutex.playback;
                playbackStartFrame =
                    p->mutex.playbackStartTime.rescaled_to(p->ioInfo.audio.sampleRate).value() -
                    otime::RationalTime(p->mutex.audioOffset, 1.0).rescaled_to(p->ioInfo.audio.sampleRate).value();
                externalTime = p->mutex.externalTime;
            }
            double speed = 0.F;
            float volume = 1.F;
            bool mute = false;
            std::chrono::steady_clock::time_point muteTimeout;
            bool reset = false;
            {
                std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                speed = p->audioMutex.speed;
                volume = p->audioMutex.volume;
                mute = p->audioMutex.mute;
                muteTimeout = p->audioMutex.muteTimeout;
                reset = p->audioMutex.reset;
                p->audioMutex.reset = false;
            }
            //std::cout << "playback: " << playback << std::endl;
            //std::cout << "playbackStartTime: " << playbackStartTime << std::endl;
            //std::cout << "reset: " << reset << std::endl;

            // Zero output audio data.
            std::memset(outputBuffer, 0, nFrames * p->audioThread.info.getByteCount());

            switch (playback)
            {
            case Playback::Forward:
            case Playback::Reverse:
            {
                // Flush the audio converter and buffer when the RtAudio
                // playback is reset.
                if (reset)
                {
                    if (p->audioThread.convert)
                    {
                        p->audioThread.convert->flush();
                    }
                    p->audioThread.buffer.clear();
                    p->audioThread.rtAudioCurrentFrame = 0;
                    p->audioThread.backwardsSize =
                        std::numeric_limits<size_t>::max();
                }

                // Create the audio converter.
                if (!p->audioThread.convert ||
                    (p->audioThread.convert && p->audioThread.convert->getInputInfo() != p->ioInfo.audio))
                {
                    p->audioThread.convert = audio::AudioConvert::create(
                        p->ioInfo.audio,
                        p->audioThread.info);
                }

                // Fill the audio buffer.
                if (p->ioInfo.audio.sampleRate > 0)
                {
                    
                    const bool backwards = playback == Playback::Reverse;
                    int64_t frame = playbackStartFrame;
                    const int64_t frameOffset = otime::RationalTime(
                        p->audioThread.rtAudioCurrentFrame + audio::getSampleCount(p->audioThread.buffer),
                        p->audioThread.info.sampleRate).rescaled_to(p->ioInfo.audio.sampleRate).value();

                    if (backwards)
                    {
                        frame -=  frameOffset;
                    }
                    else
                    {
                        frame += frameOffset;
                    }

                    int64_t seconds = p->ioInfo.audio.sampleRate > 0 ? (frame / p->ioInfo.audio.sampleRate) : 0;
                    int64_t offset = frame - seconds * p->ioInfo.audio.sampleRate;
                    //std::cout << "frame:   " << frame   << std::endl;
                    //std::cout << "seconds: " << seconds << std::endl;
                    //std::cout << "offset:  " << offset  << std::endl;
                    while (audio::getSampleCount(p->audioThread.buffer) < nFrames)
                    {
                        // std::cout << "\tseconds: " << seconds;
                        // std::cout << "\toffset: " << offset;
                        AudioData audioData;
                        {
                            std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                            const auto j = p->audioMutex.audioDataCache.find(seconds);
                            if (j != p->audioMutex.audioDataCache.end())
                            {
                                audioData = j->second;
                            }
                        }
                        if (audioData.layers.empty())
                        {
                            break;
                        }
                        
                        std::vector<std::shared_ptr<audio::Audio> > audios;
                        std::vector<const uint8_t*> audioDataP;
                        for (const auto& layer : audioData.layers)
                        {
                            if (layer.audio && layer.audio->getInfo() == p->ioInfo.audio)
                            {
                                auto audio = layer.audio;
                                if (backwards)
                                {
                                    const size_t byteCount = audio->getByteCount();
                                    const size_t sampleCount = audio->getSampleCount();
                                    auto tmp = audio::Audio::create(p->ioInfo.audio, sampleCount);
                                    tmp->zero();
                                    std::memcpy(tmp->getData(), audio->getData(), byteCount );
                                    audio = tmp;
                                    audios.push_back(audio);
                                }
                                audioDataP.push_back(
                                    audio->getData() +
                                    (offset * p->ioInfo.audio.getByteCount()));
                            }
                        }

                        size_t size = std::min(
                            getAudioBufferFrameCount(p->playerOptions.audioBufferFrameCount),
                            static_cast<size_t>(p->ioInfo.audio.sampleRate - offset));

                        if (backwards)
                        {
                            if ( p->audioThread.backwardsSize < size )
                                size = p->audioThread.backwardsSize;

                            audio::reverse(
                                const_cast<uint8_t**>(audioDataP.data()),
                                audioDataP.size(),
                                size,
                                p->ioInfo.audio.channelCount,
                                p->ioInfo.audio.dataType);
                        }
                        
                        auto tmp = audio::Audio::create(p->ioInfo.audio, size);
                        tmp->zero();
                        audio::mix(
                            audioDataP.data(),
                            audioDataP.size(),
                            tmp->getData(),
                            volume,
                            size,
                            p->ioInfo.audio.channelCount,
                            p->ioInfo.audio.dataType);

                        if (p->audioThread.convert)
                        {
                            const auto convertedAudio = p->audioThread.convert->convert(tmp);
                            p->audioThread.buffer.push_back(convertedAudio);
                        }

                        if (backwards)
                        {
                            offset -= size;
                            if (offset < 0)
                            {
                                offset += p->ioInfo.audio.sampleRate;
                                seconds -= 1;
                            }
                            p->audioThread.backwardsSize = size;
                        }
                        else
                        {
                            offset += size;
                            if (offset >= p->ioInfo.audio.sampleRate)
                            {
                                offset -= p->ioInfo.audio.sampleRate;
                                seconds += 1;
                            }
                        }

                        //std::cout << std::endl;
                    }
                    
                }

                // Copy audio data to RtAudio.
                const auto now = std::chrono::steady_clock::now();
                if (speed == p->timeline->getTimeRange().duration().rate() &&
                    !externalTime &&
                    !mute &&
                    now >= muteTimeout &&
                    nFrames <= getSampleCount(p->audioThread.buffer))
                {
                    audio::copy(
                        p->audioThread.buffer,
                        reinterpret_cast<uint8_t*>(outputBuffer),
                        nFrames * p->audioThread.info.getByteCount());
                }

                // Update the audio frame.
                p->audioThread.rtAudioCurrentFrame += nFrames;

                break;
            }
            default: break;
            }

            return 0;
        }

        void TimelinePlayer::Private::rtAudioErrorCallback(
            RtAudioError::Type type,
            const std::string& errorText)
        {}
#endif // TLRENDER_AUDIO

        void TimelinePlayer::Private::log(const std::shared_ptr<system::Context>& context)
        {
            const std::string id = string::Format("tl::timeline::TimelinePlayer {0}").arg(this);

            // Get mutex protected values.
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            uint16_t videoLayer = 0;
            PlayerCacheInfo cacheInfo;
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                currentTime = mutex.currentTime;
                inOutRange = mutex.inOutRange;
                videoLayer = mutex.videoLayer;
                cacheInfo = mutex.cacheInfo;
            }
            size_t audioDataCacheSize = 0;
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioDataCacheSize = audioMutex.audioDataCache.size();
            }

            // Create an array of characters to draw the timeline.
            const auto& timeRange = timeline->getTimeRange();
            const size_t lineLength = 80;
            std::string currentTimeDisplay(lineLength, '.');
            double n = (currentTime - timeRange.start_time()).value() / timeRange.duration().value();
            size_t index = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
            if (index < currentTimeDisplay.size())
            {
                currentTimeDisplay[index] = 'T';
            }

            // Create an array of characters to draw the cached video frames.
            std::string cachedVideoFramesDisplay(lineLength, '.');
            for (const auto& i : cacheInfo.videoFrames)
            {
                n = (i.start_time() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t0 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t1 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
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
                const size_t t0 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t1 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                for (size_t j = t0; j <= t1; ++j)
                {
                    if (j < cachedAudioFramesDisplay.size())
                    {
                        cachedAudioFramesDisplay[j] = 'A';
                    }
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
