// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>
#include <feather-tk/core/Time.h>

#include <opentimelineio/transition.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            const std::chrono::milliseconds timeout(5);
        }

        bool Timeline::Private::getVideoInfo(const OTIO_NS::Composable* composable)
        {
            if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first video clip defines the video information for the timeline.
                    if (auto read = getRead(clip, options.ioOptions))
                    {
                        const io::Info& ioInfo = read->getInfo().get();
                        this->ioInfo.video = ioInfo.video;
                        this->ioInfo.videoTime = ioInfo.videoTime;
                        this->ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
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

        bool Timeline::Private::getAudioInfo(const OTIO_NS::Composable* composable)
        {
            if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first audio clip defines the audio information for the timeline.
                    if (auto read = getRead(clip, options.ioOptions))
                    {
                        const io::Info& ioInfo = read->getInfo().get();
                        this->ioInfo.audio = ioInfo.audio;
                        this->ioInfo.audioTime = ioInfo.audioTime;
                        this->ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
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
            const auto t0 = std::chrono::steady_clock::now();

            requests();

            // Logging.
            auto t1 = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = t1 - thread.logTimer;
            if (diff.count() > 10.F)
            {
                thread.logTimer = t1;
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
                        ftk::Format("tl::timeline::Timeline {0}").arg(this),
                        ftk::Format(
                        "\n"
                        "    Path: {0}\n"
                        "    Video requests: {1}, {2} in-progress, {3} max\n"
                        "    Audio requests: {4}, {5} in-progress, {6} max").
                        arg(path.get()).
                        arg(videoRequestsSize).
                        arg(thread.videoRequestsInProgress.size()).
                        arg(options.videoRequestMax).
                        arg(audioRequestsSize).
                        arg(thread.audioRequestsInProgress.size()).
                        arg(options.audioRequestMax));
                }
                t1 = std::chrono::steady_clock::now();
            }

            // Sleep for a bit.
            ftk::sleep(timeout, t0, t1);
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
                            !mutex.videoRequests.empty() ||
                            !thread.videoRequestsInProgress.empty() ||
                            !mutex.audioRequests.empty() ||
                            !thread.audioRequestsInProgress.empty();
                    });
                while (!mutex.videoRequests.empty() &&
                    (thread.videoRequestsInProgress.size() + newVideoRequests.size()) < options.videoRequestMax)
                {
                    newVideoRequests.push_back(mutex.videoRequests.front());
                    mutex.videoRequests.pop_front();
                }
                while (!mutex.audioRequests.empty() &&
                    (thread.audioRequestsInProgress.size() + newAudioRequests.size()) < options.audioRequestMax)
                {
                    newAudioRequests.push_back(mutex.audioRequests.front());
                    mutex.audioRequests.pop_front();
                }
            }

            // Traverse the timeline for new video requests.
            for (auto& request : newVideoRequests)
            {
                for (const auto& otioTrack : otioTimeline->video_tracks())
                {
                    if (otioTrack->enabled())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem = dynamic_cast<OTIO_NS::Item*>(otioChild.value))
                            {
                                const auto requestTime = request->time - timeRange.start_time();
                                OTIO_NS::ErrorStatus errorStatus;
                                const auto range = otioItem->trimmed_range_in_parent(&errorStatus);
                                if (range.has_value() && range.value().contains(requestTime))
                                {
                                    VideoLayerData videoData;
                                    try
                                    {
                                        if (auto otioClip = dynamic_cast<const OTIO_NS::Clip*>(otioItem))
                                        {
                                            videoData.image = readVideo(otioClip, requestTime, request->options);
                                        }
                                        const auto neighbors = otioTrack->neighbors_of(otioItem, &errorStatus);
                                        if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.second.value))
                                        {
                                            if (requestTime > range.value().end_time_inclusive() - otioTransition->in_offset())
                                            {
                                                videoData.transition = toTransition(otioTransition->transition_type());
                                                videoData.transitionValue = transitionValue(
                                                    requestTime.value(),
                                                    range.value().end_time_inclusive().value() - otioTransition->in_offset().value(),
                                                    range.value().end_time_inclusive().value() + otioTransition->out_offset().value() + 1.0);
                                                const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                                if (const auto otioClipB = dynamic_cast<OTIO_NS::Clip*>(transitionNeighbors.second.value))
                                                {
                                                    videoData.imageB = readVideo(otioClipB, requestTime, request->options);
                                                }
                                            }
                                        }
                                        if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.first.value))
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
                                                if (const auto otioClipB = dynamic_cast<OTIO_NS::Clip*>(transitionNeighbors.first.value))
                                                {
                                                    videoData.image = readVideo(otioClipB, requestTime, request->options);
                                                }
                                            }
                                        }
                                    }
                                    catch (const std::exception&)
                                    {
                                        //! \todo How should this be handled?
                                    }
                                    request->layerData.push_back(std::move(videoData));
                                }
                            }
                        }
                    }
                }

                thread.videoRequestsInProgress.push_back(request);
            }

            // Traverse the timeline for new audio requests.
            for (auto& request : newAudioRequests)
            {
                for (const auto& otioTrack : otioTimeline->audio_tracks())
                {
                    if (otioTrack->enabled())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioClip = dynamic_cast<OTIO_NS::Clip*>(otioChild.value))
                            {
                                const auto rangeOptional = otioClip->trimmed_range_in_parent();
                                if (rangeOptional.has_value())
                                {
                                    const OTIO_NS::TimeRange clipTimeRange(
                                        rangeOptional.value().start_time().rescaled_to(1.0),
                                        rangeOptional.value().duration().rescaled_to(1.0));
                                    const double start = request->seconds -
                                        timeRange.start_time().rescaled_to(1.0).value();
                                    const OTIO_NS::TimeRange requestTimeRange = OTIO_NS::TimeRange(
                                        OTIO_NS::RationalTime(start, 1.0),
                                        OTIO_NS::RationalTime(1.0, 1.0));
                                    if (requestTimeRange.intersects(clipTimeRange))
                                    {
                                        AudioLayerData audioData;
                                        audioData.seconds = request->seconds;
                                        try
                                        {
                                            //! \bug Why is OTIO_NS::TimeRange::clamped() not giving us the
                                            //! result we expect?
                                            //audioData.timeRange = requestTimeRange.clamped(clipTimeRange);
                                            const double start = std::max(
                                                clipTimeRange.start_time().value(),
                                                requestTimeRange.start_time().value());
                                            const double end = std::min(
                                                clipTimeRange.start_time().value() + clipTimeRange.duration().value(),
                                                requestTimeRange.start_time().value() + requestTimeRange.duration().value());
                                            audioData.timeRange = OTIO_NS::TimeRange(
                                                OTIO_NS::RationalTime(start, 1.0),
                                                OTIO_NS::RationalTime(end - start, 1.0));
                                            audioData.audio = readAudio(otioClip, audioData.timeRange, request->options);
                                        }
                                        catch (const std::exception&)
                                        {
                                            //! \todo How should this be handled?
                                        }
                                        request->layerData.push_back(std::move(audioData));
                                    }
                                }
                            }
                        }
                    }
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
                    if (!ioInfo.video.empty())
                    {
                        data.size = ioInfo.video.front().size;
                    }
                    data.time = (*videoRequestIt)->time;
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
                    for (auto& j : (*audioRequestIt)->layerData)
                    {
                        AudioLayer layer;
                        if (j.audio.valid())
                        {
                            const auto audioData = j.audio.get();
                            if (audioData.audio)
                            {
                                layer.audio = padAudioToOneSecond(audioData.audio, j.seconds, j.timeRange);
                            }
                        }
                        data.layers.push_back(layer);
                    }
                    if (data.layers.empty())
                    {
                        auto audio = audio::Audio::create(ioInfo.audio, ioInfo.audio.sampleRate);
                        audio->zero();
                        data.layers.push_back({ audio });
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

        namespace
        {
            std::string getKey(const file::Path& path)
            {
                std::vector<std::string> out;
                out.push_back(path.get());
                out.push_back(path.getNumber());
                return ftk::join(out, ';');
            }
        }

        std::shared_ptr<io::IRead> Timeline::Private::getRead(
            const OTIO_NS::Clip* clip,
            const io::Options& ioOptions)
        {
            std::shared_ptr<io::IRead> out;
            const auto path = timeline::getPath(
                clip->media_reference(),
                this->path.getDirectory(),
                options.pathOptions);
            const std::string key = getKey(path);
            if (!readCache.get(key, out))
            {
                if (auto context = this->context.lock())
                {
                    const auto memoryRead = getMemoryRead(clip->media_reference());
                    io::Options options = ioOptions;
                    options["SequenceIO/DefaultSpeed"] = ftk::Format("{0}").arg(timeRange.duration().rate());
                    const auto ioSystem = context->getSystem<io::ReadSystem>();
                    out = ioSystem->read(path, memoryRead, options);
                    readCache.add(key, out);
                }
            }
            return out;
        }

        std::future<io::VideoData> Timeline::Private::readVideo(
            const OTIO_NS::Clip* clip,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            std::future<io::VideoData> out;
            io::Options optionsMerged = io::merge(options, this->options.ioOptions);
            optionsMerged["USD/CameraName"] = clip->name();
            auto read = getRead(clip, optionsMerged);
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                OTIO_NS::TimeRange availableRange = clip->available_range();
                OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                if (this->options.compat &&
                    availableRange.start_time() > ioInfo.videoTime.start_time())
                {
                    //! \bug If the available range is greater than the media time,
                    //! assume the media time is wrong (e.g., Picchu) and
                    //! compensate for it.
                    trimmedRange = OTIO_NS::TimeRange(
                        trimmedRange.start_time() - availableRange.start_time(),
                        trimmedRange.duration());
                }
                const auto mediaTime = timeline::toVideoMediaTime(
                    time,
                    timeRangeOpt.value(),
                    trimmedRange,
                    ioInfo.videoTime.duration().rate());
                out = read->readVideo(mediaTime, optionsMerged);
            }
            return out;
        }

        std::future<io::AudioData> Timeline::Private::readAudio(
            const OTIO_NS::Clip* clip,
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            std::future<io::AudioData> out;
            io::Options optionsMerged = io::merge(options, this->options.ioOptions);
            auto read = getRead(clip, optionsMerged);
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                if (this->options.compat &&
                    trimmedRange.start_time() < ioInfo.audioTime.start_time())
                {
                    //! \bug If the trimmed range is less than the media time,
                    //! assume the media time is wrong (e.g., ALab trailer) and
                    //! compensate for it.
                    trimmedRange = OTIO_NS::TimeRange(
                        ioInfo.audioTime.start_time() + trimmedRange.start_time(),
                        trimmedRange.duration());
                }
                const auto mediaRange = timeline::toAudioMediaTime(
                    timeRange,
                    timeRangeOpt.value(),
                    trimmedRange,
                    ioInfo.audio.sampleRate);
                out = read->readAudio(mediaRange, optionsMerged);
            }
            return out;
        }

        std::shared_ptr<audio::Audio> Timeline::Private::padAudioToOneSecond(
            const std::shared_ptr<audio::Audio>& audio,
            double seconds,
            const OTIO_NS::TimeRange& timeRange)
        {
            std::list<std::shared_ptr<audio::Audio> > list;
            const double s = seconds - this->timeRange.start_time().rescaled_to(1.0).value();
            if (timeRange.start_time().value() > s)
            {
                const OTIO_NS::RationalTime t =
                    timeRange.start_time() - OTIO_NS::RationalTime(s, 1.0);
                const OTIO_NS::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence = audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            list.push_back(audio);
            if (timeRange.end_time_exclusive().value() < s + 1.0)
            {
                const OTIO_NS::RationalTime t =
                    OTIO_NS::RationalTime(s + 1.0, 1.0) - timeRange.end_time_exclusive();
                const OTIO_NS::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence = audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            size_t sampleCount = audio::getSampleCount(list);
            auto out = audio::Audio::create(audio->getInfo(), sampleCount);
            audio::move(list, out->getData(), sampleCount);
            return out;
        }
    }
}
