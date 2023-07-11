// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/StringFormat.h>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace timeline
    {
        bool Timeline::Private::getVideoInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first video clip defines the video information for the timeline.
                    auto item = getRead(clip, options.ioOptions);
                    if (item.read)
                    {
                        this->ioInfo.video = item.ioInfo.video;
                        this->ioInfo.videoTime = item.ioInfo.videoTime;
                        this->ioInfo.tags.insert(item.ioInfo.tags.begin(), item.ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const otio::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (getVideoInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        bool Timeline::Private::getAudioInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first audio clip defines the audio information for the timeline.
                    auto item = getRead(clip, options.ioOptions);
                    if (item.read)
                    {
                        this->ioInfo.audio = item.ioInfo.audio;
                        this->ioInfo.audioTime = item.ioInfo.audioTime;
                        this->ioInfo.tags.insert(item.ioInfo.tags.begin(), item.ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const otio::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (getAudioInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        float Timeline::Private::transitionValue(double frame, double in, double out) const
        {
            return (frame - in) / (out - in);
        }

        void Timeline::Private::tick()
        {
            requests();

            // Logging.
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - thread.logTimer;
            if (diff.count() > 10.F)
            {
                thread.logTimer = now;
                if (auto context = this->context.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(mutex.mutex);
                        videoRequestsSize = mutex.videoRequests.size();
                        audioRequestsSize = mutex.audioRequests.size();
                    }
                    auto logSystem = context->getLogSystem();
                    logSystem->print(
                        string::Format("tl::timeline::Timeline {0}").arg(this),
                        string::Format(
                        "\n"
                        "    Path: {0}\n"
                        "    Video requests: {1}, {2} in-progress, {3} max\n"
                        "    Audio requests: {4}, {5} in-progress, {6} max\n"
                        "    Read cache: {7}").
                        arg(path.get()).
                        arg(videoRequestsSize).
                        arg(thread.videoRequestsInProgress.size()).
                        arg(options.videoRequestCount).
                        arg(audioRequestsSize).
                        arg(thread.audioRequestsInProgress.size()).
                        arg(options.audioRequestCount).
                        arg(readCache->getCount()));
                }
            }

            // Sleep for a bit...
            time::sleep(std::chrono::milliseconds(1));
        }

        void Timeline::Private::requests()
        {
            // Gather requests.
            std::list<std::shared_ptr<VideoRequest> > newVideoRequests;
            std::list<std::shared_ptr<AudioRequest> > newAudioRequests;
            {
                std::unique_lock<std::mutex> lock(mutex.mutex);
                thread.cv.wait_for(
                    lock,
                    options.requestTimeout,
                    [this]
                    {
                        return
                            mutex.otioTimeline.value ||
                            !mutex.videoRequests.empty() ||
                            !thread.videoRequestsInProgress.empty() ||
                            !mutex.audioRequests.empty() ||
                            !thread.audioRequestsInProgress.empty();
                    });
                if (mutex.otioTimeline.value)
                {
                    thread.otioTimeline = mutex.otioTimeline;
                    mutex.otioTimeline = nullptr;
                    mutex.otioTimelineChanged = true;
                }
                while (!mutex.videoRequests.empty() &&
                    (thread.videoRequestsInProgress.size() + newVideoRequests.size()) < options.videoRequestCount)
                {
                    newVideoRequests.push_back(mutex.videoRequests.front());
                    mutex.videoRequests.pop_front();
                }
                while (!mutex.audioRequests.empty() &&
                    (thread.audioRequestsInProgress.size() + newAudioRequests.size()) < options.audioRequestCount)
                {
                    newAudioRequests.push_back(mutex.audioRequests.front());
                    mutex.audioRequests.pop_front();
                }
            }

            // Traverse the timeline for new video requests.
            for (auto& request : newVideoRequests)
            {
                try
                {
                    for (const auto& otioTrack : thread.otioTimeline->video_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem = dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const auto requestTime = request->time - timeRange.start_time();
                                otio::ErrorStatus errorStatus;
                                const auto range = otioItem->trimmed_range_in_parent(&errorStatus);
                                if (range.has_value() && range.value().contains(requestTime))
                                {
                                    VideoLayerData videoData;
                                    if (auto otioClip = dynamic_cast<const otio::Clip*>(otioItem))
                                    {
                                        videoData.image = readVideo(otioTrack, otioClip, requestTime, request->videoLayer);
                                    }
                                    const auto neighbors = otioTrack->neighbors_of(otioItem, &errorStatus);
                                    if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.second.value))
                                    {
                                        if (requestTime > range.value().end_time_inclusive() - otioTransition->in_offset())
                                        {
                                            videoData.transition = toTransition(otioTransition->transition_type());
                                            videoData.transitionValue = transitionValue(
                                                requestTime.value(),
                                                range.value().end_time_inclusive().value() - otioTransition->in_offset().value(),
                                                range.value().end_time_inclusive().value() + otioTransition->out_offset().value() + 1.0);
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.second.value))
                                            {
                                                videoData.imageB = readVideo(otioTrack, otioClipB, requestTime, request->videoLayer);
                                            }
                                        }
                                    }
                                    if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.first.value))
                                    {
                                        if (requestTime < range.value().start_time() + otioTransition->out_offset())
                                        {
                                            std::swap(videoData.image, videoData.imageB);
                                            videoData.transition = toTransition(otioTransition->transition_type());
                                            videoData.transitionValue = transitionValue(
                                                requestTime.value(),
                                                range.value().start_time().value() - otioTransition->in_offset().value() - 1.0,
                                                range.value().start_time().value() + otioTransition->out_offset().value());
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.first.value))
                                            {
                                                videoData.image = readVideo(otioTrack, otioClipB, requestTime, request->videoLayer);
                                            }
                                        }
                                    }
                                    request->layerData.push_back(std::move(videoData));
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                thread.videoRequestsInProgress.push_back(request);
            }

            // Traverse the timeline for new audio requests.
            for (auto& request : newAudioRequests)
            {
                try
                {
                    for (const auto& otioTrack : thread.otioTimeline->audio_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem = dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const auto rangeOptional = otioItem->trimmed_range_in_parent();
                                if (rangeOptional.has_value())
                                {
                                    const otime::TimeRange clipTimeRange(
                                        rangeOptional.value().start_time().rescaled_to(1.0),
                                        rangeOptional.value().duration().rescaled_to(1.0));
                                    const double start = request->seconds -
                                        timeRange.start_time().rescaled_to(1.0).value();
                                    const otime::TimeRange requestTimeRange = otime::TimeRange(
                                        otime::RationalTime(start, 1.0),
                                        otime::RationalTime(1.0, 1.0));
                                    if (requestTimeRange.intersects(clipTimeRange))
                                    {
                                        AudioLayerData audioData;
                                        audioData.seconds = request->seconds;
                                        audioData.timeRange = requestTimeRange.clamped(clipTimeRange);
                                        if (auto otioClip = dynamic_cast<const otio::Clip*>(otioItem))
                                        {
                                            audioData.audio = readAudio(otioTrack, otioClip, requestTimeRange);
                                        }
                                        request->layerData.push_back(std::move(audioData));
                                    }
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                thread.audioRequestsInProgress.push_back(request);
            }

            // Check for finished video requests.
            auto videoRequestIt = thread.videoRequestsInProgress.begin();
            while (videoRequestIt != thread.videoRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*videoRequestIt)->layerData)
                {
                    if (i.image.valid())
                    {
                        valid &= i.image.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                    if (i.imageB.valid())
                    {
                        valid &= i.imageB.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                }
                if (valid)
                {
                    VideoData data;
                    data.time = (*videoRequestIt)->time;
                    try
                    {
                        for (auto& j : (*videoRequestIt)->layerData)
                        {
                            VideoLayer layer;
                            if (j.image.valid())
                            {
                                layer.image = j.image.get().image;
                            }
                            if (j.imageB.valid())
                            {
                                layer.imageB = j.imageB.get().image;
                            }
                            layer.transition = j.transition;
                            layer.transitionValue = j.transitionValue;
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    (*videoRequestIt)->promise.set_value(data);
                    videoRequestIt = thread.videoRequestsInProgress.erase(videoRequestIt);
                    continue;
                }
                ++videoRequestIt;
            }

            // Check for finished audio requests.
            auto audioRequestIt = thread.audioRequestsInProgress.begin();
            while (audioRequestIt != thread.audioRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*audioRequestIt)->layerData)
                {
                    if (i.audio.valid())
                    {
                        valid &= i.audio.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                }
                if (valid)
                {
                    AudioData data;
                    data.seconds = (*audioRequestIt)->seconds;
                    try
                    {
                        for (auto& j : (*audioRequestIt)->layerData)
                        {
                            AudioLayer layer;
                            if (j.audio.valid())
                            {
                                const auto audioData = j.audio.get();
                                if (audioData.audio)
                                {
                                    trimAudio(audioData.audio, j.seconds, j.timeRange);
                                }
                                layer.audio = audioData.audio;
                            }
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    (*audioRequestIt)->promise.set_value(data);
                    audioRequestIt = thread.audioRequestsInProgress.erase(audioRequestIt);
                    continue;
                }
                ++audioRequestIt;
            }
        }

        void Timeline::Private::finishRequests()
        {
            {
                std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
                std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
                {
                    std::unique_lock<std::mutex> lock(mutex.mutex);
                    mutex.stopped = true;
                    videoRequests = std::move(mutex.videoRequests);
                    audioRequests = std::move(mutex.audioRequests);
                }
                videoRequests.insert(
                    videoRequests.begin(),
                    thread.videoRequestsInProgress.begin(),
                    thread.videoRequestsInProgress.end());
                thread.videoRequestsInProgress.clear();
                audioRequests.insert(
                    audioRequests.begin(),
                    thread.audioRequestsInProgress.begin(),
                    thread.audioRequestsInProgress.end());
                thread.audioRequestsInProgress.clear();
                for (auto& request : videoRequests)
                {
                    VideoData data;
                    data.time = request->time;
                    for (auto& i : request->layerData)
                    {
                        VideoLayer layer;
                        if (i.image.valid())
                        {
                            layer.image = i.image.get().image;
                        }
                        if (i.imageB.valid())
                        {
                            layer.imageB = i.imageB.get().image;
                        }
                        layer.transition = i.transition;
                        layer.transitionValue = i.transitionValue;
                        data.layers.push_back(layer);
                    }
                    request->promise.set_value(data);
                }
                for (auto& request : audioRequests)
                {
                    AudioData data;
                    data.seconds = request->seconds;
                    for (auto& i : request->layerData)
                    {
                        AudioLayer layer;
                        if (i.audio.valid())
                        {
                            layer.audio = i.audio.get().audio;
                        }
                        data.layers.push_back(layer);
                    }
                    request->promise.set_value(data);
                }
            }
        }

        ReadCacheItem Timeline::Private::getRead(
            const otio::Clip* clip,
            const io::Options& ioOptions)
        {
            ReadCacheItem out;
            const auto path = timeline::getPath(
                clip->media_reference(),
                this->path.getDirectory(),
                options.pathOptions);
            if (!readCache->get(path, out))
            {
                if (auto context = this->context.lock())
                {
                    const auto path = timeline::getPath(
                        clip->media_reference(),
                        this->path.getDirectory(),
                        options.pathOptions);
                    const auto memoryRead = getMemoryRead(clip->media_reference());
                    io::Options options = ioOptions;
                    options["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(timeRange.duration().rate());
                    otio::ErrorStatus error;
                    otime::RationalTime startTime = time::invalidTime;
                    const otime::TimeRange availableRange = clip->available_range(&error);
                    if (!otio::is_error(error))
                    {
                        startTime = availableRange.start_time();
                    }
                    else if (clip->source_range().has_value())
                    {
                        startTime = clip->source_range().value().start_time();
                    }
                    options["FFmpeg/StartTime"] = string::Format("{0}").arg(startTime);
                    const auto ioSystem = context->getSystem<io::System>();
                    out.read = ioSystem->read(path, memoryRead, options);
                    if (out.read)
                    {
                        out.ioInfo = out.read->getInfo().get();
                        readCache->add(out);
                        context->log(
                            string::Format("tl::timeline::Timeline {0}").arg(this),
                            string::Format(
                                "\n"
                                "    Read: {0}\n"
                                "    Video: {1} {2}\n"
                                "    Video time: {3}\n"
                                "    Audio: {4} {5} {6}\n"
                                "    Audio time: {7}").
                            arg(path.get()).
                            arg(!out.ioInfo.video.empty() ? out.ioInfo.video[0].size : imaging::Size()).
                            arg(!out.ioInfo.video.empty() ? out.ioInfo.video[0].pixelType : imaging::PixelType::None).
                            arg(out.ioInfo.videoTime).
                            arg(out.ioInfo.audio.channelCount).
                            arg(out.ioInfo.audio.dataType).
                            arg(out.ioInfo.audio.sampleRate).
                            arg(out.ioInfo.audioTime));
                    }
                }
            }
            return out;
        }

        std::future<io::VideoData> Timeline::Private::readVideo(
            const otio::Track* track,
            const otio::Clip* clip,
            const otime::RationalTime& time,
            uint16_t videoLayer)
        {
            std::future<io::VideoData> out;
            ReadCacheItem item = getRead(clip, options.ioOptions);
            if (item.read)
            {
                const auto mediaTime = timeline::toVideoMediaTime(
                    time,
                    track,
                    clip,
                    item.ioInfo);
                out = item.read->readVideo(mediaTime, videoLayer);
            }
            return out;
        }

        std::future<io::AudioData> Timeline::Private::readAudio(
            const otio::Track* track,
            const otio::Clip* clip,
            const otime::TimeRange& timeRange)
        {
            std::future<io::AudioData> out;
            ReadCacheItem item = getRead(clip, options.ioOptions);
            if (item.read)
            {
                const auto mediaRange = timeline::toAudioMediaTime(
                    timeRange,
                    track,
                    clip,
                    item.ioInfo);
                out = item.read->readAudio(mediaRange);
            }
            return out;
        }

        void Timeline::Private::trimAudio(
            const std::shared_ptr<audio::Audio>& audio,
            int64_t seconds,
            const otime::TimeRange& timeRange)
        {
            const double s = seconds - this->timeRange.start_time().rescaled_to(1.0).value();
            if (timeRange.start_time().value() > s)
            {
                const otime::RationalTime t =
                    timeRange.start_time() - otime::RationalTime(s, 1.0);
                const otime::RationalTime t2 =
                    t.rescaled_to(audio->getSampleCount());
                const size_t size = t2.value() *
                    audio->getInfo().getByteCount();
                memset(audio->getData(), 0, size);
            }
            if (timeRange.end_time_exclusive().value() < s + 1)
            {
                const otime::RationalTime t =
                    timeRange.end_time_exclusive() - otime::RationalTime(s, 1.0);
                const otime::RationalTime t2 =
                    t.rescaled_to(audio->getSampleCount());
                const size_t offset = t2.value() *
                    audio->getInfo().getByteCount();
                const size_t size = audio->getByteCount() -
                    offset;
                memset(audio->getData() + offset, 0, size);
            }
        }
    }
}
