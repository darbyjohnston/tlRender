// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/StringFormat.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/transition.h>

namespace tl
{
    namespace timeline
    {
        file::Path Timeline::Private::getPath(const otio::MediaReference* ref) const
        {
            std::string url;
            file::PathOptions pathOptions = options.pathOptions;
            if (auto externalRef = dynamic_cast<const otio::ExternalReference*>(ref))
            {
                url = externalRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (auto imageSequenceRef = dynamic_cast<const otio::ImageSequenceReference*>(ref))
            {
                std::stringstream ss;
                ss << imageSequenceRef->target_url_base() <<
                    imageSequenceRef->name_prefix() <<
                    std::setfill('0') << std::setw(imageSequenceRef->frame_zero_padding()) <<
                    imageSequenceRef->start_frame() <<
                    imageSequenceRef->name_suffix();
                url = ss.str();
            }
            else if (auto rawMemoryRef = dynamic_cast<const RawMemoryReference*>(ref))
            {
                url = rawMemoryRef->target_url();
            }
            else if (auto sharedMemoryRef = dynamic_cast<const SharedMemoryReference*>(ref))
            {
                url = sharedMemoryRef->target_url();
            }
            else if (auto rawMemorySequenceRef = dynamic_cast<const RawMemorySequenceReference*>(ref))
            {
                url = rawMemorySequenceRef->target_url();
            }
            else if (auto sharedMemorySequenceRef = dynamic_cast<const SharedMemorySequenceReference*>(ref))
            {
                url = sharedMemorySequenceRef->target_url();
            }
            return timeline::getPath(url, path.getDirectory(), pathOptions);
        }

        std::vector<file::MemoryRead> Timeline::Private::getMemoryRead(const otio::MediaReference* ref)
        {
            std::vector<file::MemoryRead> out;
            if (auto rawMemoryReference =
                dynamic_cast<const RawMemoryReference*>(ref))
            {
                out.push_back(file::MemoryRead(
                    rawMemoryReference->memory(),
                    rawMemoryReference->memory_size()));
            }
            else if (auto sharedMemoryReference =
                dynamic_cast<const SharedMemoryReference*>(ref))
            {
                if (const auto& memory = sharedMemoryReference->memory())
                {
                    out.push_back(file::MemoryRead(
                        memory->data(),
                        memory->size()));
                }
            }
            else if (auto rawMemorySequenceReference =
                dynamic_cast<const RawMemorySequenceReference*>(ref))
            {
                const auto& memory = rawMemorySequenceReference->memory();
                const size_t memory_size = memory.size();
                const auto& memory_sizes = rawMemorySequenceReference->memory_sizes();
                const size_t memory_sizes_size = memory_sizes.size();
                for (size_t i = 0; i < memory_size && i < memory_sizes_size; ++i)
                {
                    out.push_back(file::MemoryRead(memory[i], memory_sizes[i]));
                }
            }
            else if (auto sharedMemorySequenceReference =
                dynamic_cast<const SharedMemorySequenceReference*>(ref))
            {
                for (const auto& memory : sharedMemorySequenceReference->memory())
                {
                    if (memory)
                    {
                        out.push_back(file::MemoryRead(memory->data(), memory->size()));
                    }
                }
            }
            return out;
        }

        bool Timeline::Private::getVideoInfo(const otio::Composable* composable)
        {
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = this->context.lock())
                {
                    // The first video clip defines the video information for the timeline.
                    io::Options ioOptions = options.ioOptions;
                    ioOptions["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(clip->duration().rate());
                    auto item = getRead(clip, ioOptions);
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
                        return !mutex.videoRequests.empty() ||
                            !thread.videoRequestsInProgress.empty() ||
                            !mutex.audioRequests.empty() ||
                            !thread.audioRequestsInProgress.empty();
                    });
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
                    for (const auto& otioTrack : otioTimeline->video_tracks())
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
                    for (const auto& otioTrack : otioTimeline->audio_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem = dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const otime::TimeRange requestTimeRange = otime::TimeRange(
                                    otime::RationalTime(request->seconds, 1.0) - timeRange.start_time().rescaled_to(1.0),
                                    otime::RationalTime(1.0, 1.0));
                                otio::ErrorStatus errorStatus;
                                const auto rangeOptional = otioItem->trimmed_range_in_parent(&errorStatus);
                                if (rangeOptional.has_value())
                                {
                                    const auto range = rangeOptional.value();
                                    if (range.intersects(requestTimeRange))
                                    {
                                        AudioLayerData audioData;
                                        const otime::TimeRange clamped = requestTimeRange.clamped(range);
                                        audioData.timeRange = otime::TimeRange::range_from_start_end_time(
                                            time::round(clamped.start_time()),
                                            time::round(clamped.end_time_exclusive()));
                                        if (auto otioClip = dynamic_cast<const otio::Clip*>(otioItem))
                                        {
                                            audioData.audio = readAudio(otioTrack, otioClip, audioData.timeRange);
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
                                layer.audio = padToOneSecond(audioData.audio, j.timeRange);
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
            const auto path = getPath(clip->media_reference());
            if (!readCache->get(path, out))
            {
                if (auto context = this->context.lock())
                {
                    const auto path = getPath(clip->media_reference());
                    const auto memoryRead = getMemoryRead(clip->media_reference());
                    io::Options options = ioOptions;
                    options["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(timeRange.duration().rate());
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
                otime::RationalTime clipTime = track->transformed_time(time, clip);
                if (auto externalReference = dynamic_cast<const otio::ExternalReference*>(clip->media_reference()))
                {
                    // If the available range start time is greater than the
                    // video end time we assume the media is missing timecode
                    // and adjust accordingly.
                    const auto availableRangeOpt = externalReference->available_range();
                    if (availableRangeOpt.has_value() &&
                        availableRangeOpt->start_time() > item.ioInfo.videoTime.end_time_inclusive())
                    {
                        clipTime -= availableRangeOpt->start_time();
                    }
                }
                const otime::RationalTime readTime = time::round(clipTime.rescaled_to(item.ioInfo.videoTime.duration().rate()));
                out = item.read->readVideo(readTime, videoLayer);
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
                otime::TimeRange clipRange = track->transformed_time_range(timeRange, clip);
                if (auto externalReference = dynamic_cast<const otio::ExternalReference*>(clip->media_reference()))
                {
                    // If the available range start time is greater than the
                    // video end time we assume the media is missing timecode
                    // and adjust accordingly.
                    const auto availableRangeOpt = externalReference->available_range();
                    if (availableRangeOpt.has_value() &&
                        availableRangeOpt->start_time() > item.ioInfo.videoTime.end_time_inclusive())
                    {
                        clipRange = otime::TimeRange(
                            clipRange.start_time() - availableRangeOpt->start_time(),
                            clipRange.duration());
                    }
                }
                const otime::TimeRange readRange(
                    time::floor(clipRange.start_time().rescaled_to(ioInfo.audio.sampleRate)),
                    time::ceil(clipRange.duration().rescaled_to(ioInfo.audio.sampleRate)));
                out = item.read->readAudio(readRange);
            }
            return out;
        }

        std::shared_ptr<audio::Audio> Timeline::Private::padToOneSecond(
            const std::shared_ptr<audio::Audio>& audio,
            const otime::TimeRange& timeRange)
        {
            std::shared_ptr<audio::Audio> out;
            if (audio)
            {
                std::list<std::shared_ptr<audio::Audio> > list;
                const auto& info = audio->getInfo();
                const otime::RationalTime offset = timeRange.start_time() -
                    time::floor(timeRange.start_time().rescaled_to(1.0)).
                    rescaled_to(info.sampleRate);
                if (offset.value() > 0.0)
                {
                    auto silence = audio::Audio::create(info, offset.value());
                    silence->zero();
                    list.push_back(silence);
                }
                list.push_back(audio);
                const size_t sampleCount = audio::getSampleCount(list);
                if (sampleCount < info.sampleRate)
                {
                    auto silence = audio::Audio::create(info, info.sampleRate - sampleCount);
                    silence->zero();
                    list.push_back(silence);
                }
                out = audio::Audio::create(info, info.sampleRate);
                audio::copy(list, out->getData(), out->getByteCount());
            }
            return out;
        }
    }
}
