// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            const double cacheFill = 0.0;
        }

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

        size_t Player::Private::getVideoCacheMax() const
        {
            // This function returns the approximate number of video frames
            // that can fit in the cache. Note that this doesn't take into
            // account clips with different sizes or multiple tracks.
            size_t byteCount = 0;
            if (thread.state.videoLayer >= 0 &&
                thread.state.videoLayer < ioInfo.video.size())
            {
                byteCount += ioInfo.video[thread.state.videoLayer].getByteCount();

                // Add byte counts from timelines that are being compared.
                for (size_t i = 0; i < thread.state.compare.size(); ++i)
                {
                    const int compareLayer = i < thread.state.compareVideoLayers.size() ?
                        thread.state.compareVideoLayers[i] :
                        thread.state.videoLayer;
                    const io::Info& compareInfo = thread.state.compare[i]->getIOInfo();
                    if (compareLayer >= 0 &&
                        compareLayer < compareInfo.video.size())
                    {
                        byteCount += compareInfo.video[compareLayer].getByteCount();
                    }
                }
            }
            return (thread.state.cacheOptions.videoGB * ftk::gigabyte) / byteCount;
        }

        size_t Player::Private::getAudioCacheMax() const
        {
            // This function returns the approximate number seconds of audio
            // that can fit in the cache. Note that this doesn't take into
            // account clips with different sizes or multiple tracks.
            return (thread.state.cacheOptions.audioGB * ftk::gigabyte) /
                (ioInfo.audio.sampleRate * ioInfo.audio.getByteCount());
        }

        std::vector<OTIO_NS::RationalTime> Player::Private::getVideoCacheTimes() const
        {
            std::vector<OTIO_NS::RationalTime> out;

            const double rate = thread.state.currentTime.rate();
            const OTIO_NS::RationalTime readAhead =
                OTIO_NS::RationalTime(thread.state.cacheOptions.readAhead, 1.0).rescaled_to(rate);
            const OTIO_NS::RationalTime readBehind =
                OTIO_NS::RationalTime(thread.state.cacheOptions.readBehind, 1.0).rescaled_to(rate);
            const OTIO_NS::RationalTime cacheAhead(thread.videoCache.getMax() * cacheFill, rate);

            switch (thread.cacheDirection)
            {
            case CacheDirection::Forward:
            {
                const OTIO_NS::RationalTime start = thread.state.currentTime - readBehind;
                const OTIO_NS::RationalTime end = thread.state.currentTime + std::max(readAhead, cacheAhead);
                const OTIO_NS::RationalTime inc(1.0, rate);
                for (OTIO_NS::RationalTime t = start; t <= end; t += inc)
                {
                    out.push_back(t);
                }
                break;
            }
            case CacheDirection::Reverse:
            {
                const OTIO_NS::RationalTime start = thread.state.currentTime + readBehind;
                const OTIO_NS::RationalTime end = thread.state.currentTime - std::max(readAhead, cacheAhead);
                const OTIO_NS::RationalTime inc(1.0, rate);
                for (OTIO_NS::RationalTime t = start; t >= end; t -= inc)
                {
                    out.push_back(t);
                }
                break;
            }
            default: break;
            }

            return out;
        }

        std::vector<int64_t> Player::Private::getAudioCacheSeconds() const
        {
            std::vector<int64_t> out;

            const int64_t c = thread.state.currentTime.rescaled_to(1.0).value();
            const int64_t readAhead = OTIO_NS::RationalTime(thread.state.cacheOptions.readAhead, 1.0).value();
            const int64_t readBehind = OTIO_NS::RationalTime(thread.state.cacheOptions.readBehind, 1.0).value();
            const int64_t cacheAhead = getAudioCacheMax() * cacheFill;

            switch (thread.cacheDirection)
            {
            case CacheDirection::Forward:
            {
                const int64_t start = c - readBehind;
                const int64_t end = c + std::max(readAhead, cacheAhead);
                for (int64_t s = start; s <= end; ++s)
                {
                    out.push_back(s);
                }
                break;
            }
            case CacheDirection::Reverse:
            {
                const int64_t start = c + readBehind;
                const int64_t end = c - std::max(readAhead, cacheAhead);
                for (int64_t s = start; s >= end; --s)
                {
                    out.push_back(s);
                }
                break;
            }
            default: break;
            }

            return out;
        }

        void Player::Private::cacheUpdate()
        {
            //std::cout << "current time: " << currentTime->get() << std::endl;

            thread.videoCache.setMax(getVideoCacheMax());
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.cache.setMax(getAudioCacheMax());
            }

            // Fill the video cache.
            if (!ioInfo.video.empty())
            {
                const auto times = getVideoCacheTimes();
                for (size_t i = 0;
                    i < times.size() &&
                    thread.videoDataRequests.size() < playerOptions.videoRequestMax;
                    ++i)
                {
                    const OTIO_NS::RationalTime t = timeline::loop(times[i], thread.state.inOutRange);
                    if (!thread.videoCache.contains(t))
                    {
                        const auto j = thread.videoDataRequests.find(t);
                        if (j == thread.videoDataRequests.end())
                        {
                            //std::cout << this << " video request: " << t << std::endl;
                            auto& requests = thread.videoDataRequests[t];
                            io::Options ioOptions2 = thread.state.ioOptions;
                            ioOptions2["Layer"] = ftk::Format("{0}").arg(thread.state.videoLayer);
                            requests.clear();
                            requests.push_back(timeline->getVideo(t, ioOptions2));

                            for (size_t k = 0; k < thread.state.compare.size(); ++k)
                            {
                                const OTIO_NS::RationalTime t2 = timeline::getCompareTime(
                                    t,
                                    timeRange,
                                    thread.state.compare[k]->getTimeRange(),
                                    thread.state.compareTime);
                                ioOptions2["Layer"] = ftk::Format("{0}").
                                    arg(k < thread.state.compareVideoLayers.size() ?
                                        thread.state.compareVideoLayers[k] :
                                        thread.state.videoLayer);
                                requests.push_back(thread.state.compare[k]->getVideo(t2, ioOptions2));
                            }
                        }
                    }
                }
                //std::cout << this << " video requests: " << thread.videoDataRequests.size() << std::endl;
            }

            // Fill the audio cache.
            if (ioInfo.audio.isValid())
            {
                const auto seconds = getAudioCacheSeconds();
                for (size_t i = 0;
                    i < seconds.size() &&
                    thread.audioDataRequests.size() < playerOptions.audioRequestMax;
                    ++i)
                {
                    const int64_t t = timeline::loop(seconds[i] + thread.state.audioOffset, thread.state.inOutRange);
                    bool found = false;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        found = audioMutex.cache.contains(t);
                    }
                    if (!found)
                    {
                        const auto j = thread.audioDataRequests.find(t);
                        if (j == thread.audioDataRequests.end())
                        {
                            auto& request = thread.audioDataRequests[t];
                            request = timeline->getAudio(t, thread.state.ioOptions);
                        }
                    }
                }
                //std::cout << this << " audio requests: " << thread.audioDataRequests.size() << std::endl;
            }

            // Check for finished video.
            bool videoCacheChanged = false;
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
                    thread.videoCache.add(time, videoDataList);
                    videoCacheChanged = true;
                    videoDataRequestsIt = thread.videoDataRequests.erase(videoDataRequestsIt);
                }
                else
                {
                    ++videoDataRequestsIt;
                }
            }

            // Check for finished audio.
            bool audioCacheChanged = false;
            auto audioDataRequestsIt = thread.audioDataRequests.begin();
            while (audioDataRequestsIt != thread.audioDataRequests.end())
            {
                const int64_t s = audioDataRequestsIt->first;
                if (audioDataRequestsIt->second.future.valid() &&
                    audioDataRequestsIt->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto audioData = audioDataRequestsIt->second.future.get();
                    audioData.seconds = s;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        audioMutex.cache.add(audioDataRequestsIt->first, audioData);
                    }
                    audioDataRequestsIt = thread.audioDataRequests.erase(audioDataRequestsIt);
                    audioCacheChanged = true;
                }
                else
                {
                    ++audioDataRequestsIt;
                }
            }

            // Update cache information.
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - thread.cacheTimer;
            if (diff.count() > .5F)
            {
                thread.cacheTimer = now;

                std::vector<OTIO_NS::RationalTime> videoCacheFrames;
                for (const auto& key : thread.videoCache.getKeys())
                {
                    videoCacheFrames.push_back(key);
                }
                const size_t videoCacheMax = thread.videoCache.getMax();
                const float videoCachePercentage = videoCacheMax > 0 ?
                    (thread.videoCache.getSize() / static_cast<float>(videoCacheMax) * 100.F) :
                    0.F;

                size_t audioCacheMax = 0;
                size_t audioCacheSize = 0;
                std::vector<int64_t> audioCacheKeys;
                {
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    audioCacheMax = audioMutex.cache.getMax();
                    audioCacheSize = audioMutex.cache.getSize();
                    audioCacheKeys = audioMutex.cache.getKeys();
                }
                std::vector<OTIO_NS::RationalTime> audioCacheFrames;
                for (const auto& key : audioCacheKeys)
                {
                    audioCacheFrames.push_back(OTIO_NS::RationalTime(key, 1.0));
                }
                const float audioCachePercentage = audioCacheMax > 0 ?
                    (audioCacheSize / static_cast<float>(audioCacheMax) * 100.F) :
                    0.F;

                auto videoCacheRanges = toRanges(videoCacheFrames);
                auto audioCacheRanges = toRanges(audioCacheFrames);
                for (auto& i : audioCacheRanges)
                {
                    i = OTIO_NS::TimeRange(
                        i.start_time().rescaled_to(timeRange.duration().rate()).floor(),
                        i.duration().rescaled_to(timeRange.duration().rate()).ceil());
                }
                {
                    std::unique_lock<std::mutex> lock(mutex.mutex);
                    mutex.cacheInfo.videoPercentage = videoCachePercentage;
                    mutex.cacheInfo.audioPercentage = audioCachePercentage;
                    mutex.cacheInfo.video = videoCacheRanges;
                    mutex.cacheInfo.audio = audioCacheRanges;
                }
            }
        }

        void Player::Private::playbackReset(const OTIO_NS::RationalTime& time)
        {
            noAudio.playbackTimer = std::chrono::steady_clock::now();
            noAudio.start = time;
        }

        void Player::Private::log(const std::shared_ptr<ftk::Context>& context)
        {
            const std::string id = ftk::Format("tl::timeline::Player {0}").arg(this);

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
            const size_t videoCacheMax = thread.videoCache.getMax();
            const size_t videoCacheSize = thread.videoCache.getSize();
            size_t audioCacheMax = 0;
            size_t audioCacheSize = 0;
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioCacheMax = audioMutex.cache.getMax();
                audioCacheSize = audioMutex.cache.getSize();
            }

            // Create an array of characters to draw the timeline.
            const size_t lineLength = 80;
            std::string currentTimeDisplay(lineLength, '.');
            double n = (currentTime - timeRange.start_time()).value() / timeRange.duration().value();
            size_t index = ftk::clamp(n, 0.0, 1.0) * (lineLength - 1);
            if (index < currentTimeDisplay.size())
            {
                currentTimeDisplay[index] = 'T';
            }

            // Create an array of characters to draw the cached video frames.
            std::string cachedVideoFramesDisplay(lineLength, '.');
            for (const auto& i : cacheInfo.video)
            {
                n = (i.start_time() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t0 = ftk::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t1 = ftk::clamp(n, 0.0, 1.0) * (lineLength - 1);
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
                const size_t t0 = ftk::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeRange.start_time()).value() / timeRange.duration().value();
                const size_t t1 = ftk::clamp(n, 0.0, 1.0) * (lineLength - 1);
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
                ioOptionStrings.push_back(ftk::Format("{0}:{1}").arg(i.first).arg(i.second));
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(id, ftk::Format(
                "\n"
                "    Path: {0}\n"
                "    Current time: {1}\n"
                "    In/out range: {2}\n"
                "    I/O options: {3}\n"
                "    Video cache: {4}% {5}GB\n"
                "    Audio cache: {6}% {7}GB\n"
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
                arg(ftk::join(ioOptionStrings, ", ")).
                arg(videoCacheMax > 0 ? (videoCacheSize / static_cast<double>(videoCacheMax) * 100.0) : 0.0).
                arg(cacheOptions->get().videoGB).
                arg(audioCacheMax > 0 ? (audioCacheSize / static_cast<double>(audioCacheMax) * 100.0) : 0.0).
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
