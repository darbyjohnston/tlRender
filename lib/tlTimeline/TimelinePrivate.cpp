// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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
            if (auto externalRef = dynamic_cast<const otio::ExternalReference*>(ref))
            {
                url = externalRef->target_url();
            }
            else if (auto imageSequenceRef = dynamic_cast<const otio::ImageSequenceReference*>(ref))
            {
                std::stringstream ss;
                ss << imageSequenceRef->target_url_base() <<
                    imageSequenceRef->name_prefix() <<
                    std::setfill('0') << std::setw(imageSequenceRef->frame_zero_padding()) <<
                    imageSequenceRef->start_frame() <<
                    imageSequenceRef->name_suffix();
                ss >> url;
            }
            else if (auto memoryRef = dynamic_cast<const MemoryReference*>(ref))
            {
                url = memoryRef->target_url();
            }
            else if (auto memorySequenceRef = dynamic_cast<const MemorySequenceReference*>(ref))
            {
                url = memorySequenceRef->target_url();
            }
            return timeline::getPath(url, path.getDirectory(), options.pathOptions);
        }

        std::vector<io::MemoryRead> Timeline::Private::getMemoryRead(const otio::MediaReference* ref)
        {
            std::vector<io::MemoryRead> out;
            if (auto memoryReference =
                dynamic_cast<const MemoryReference*>(ref))
            {
                out.push_back(io::MemoryRead(
                    memoryReference->memory_ptr(),
                    memoryReference->memory_size()));
            }
            else if (auto memorySequenceReference =
                dynamic_cast<const MemorySequenceReference*>(ref))
            {
                const auto& memory_ptrs = memorySequenceReference->memory_ptrs();
                const size_t memory_ptrs_size = memory_ptrs.size();
                const auto& memory_sizes = memorySequenceReference->memory_sizes();
                const size_t memory_sizes_size = memory_sizes.size();
                for (size_t i = 0; i < memory_ptrs_size && i < memory_sizes_size; ++i)
                {
                    out.push_back(io::MemoryRead(memory_ptrs[i], memory_sizes[i]));
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
                    const file::Path path = getPath(clip->media_reference());
                    const auto memoryRead = getMemoryRead(clip->media_reference());
                    io::Options ioOptions = options.ioOptions;
                    ioOptions["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(clip->duration().rate());
                    if (auto read = context->getSystem<io::System>()->read(path, memoryRead, ioOptions))
                    {
                        const auto ioInfo = read->getInfo().get();
                        this->ioInfo.video = ioInfo.video;
                        this->ioInfo.videoTime = ioInfo.videoTime;
                        this->ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
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
                    const auto path = getPath(clip->media_reference());
                    const auto memoryRead = getMemoryRead(clip->media_reference());
                    io::Options ioOptions = options.ioOptions;
                    if (auto read = context->getSystem<io::System>()->read(path, memoryRead, ioOptions))
                    {
                        const auto ioInfo = read->getInfo().get();
                        this->ioInfo.audio = ioInfo.audio;
                        this->ioInfo.audioTime = ioInfo.audioTime;
                        this->ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
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
            stopReaders();
            delReaders();

            // Logging.
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - logTimer;
            if (diff.count() > 10.F)
            {
                logTimer = now;
                if (auto context = this->context.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        videoRequestsSize = videoRequests.size();
                        audioRequestsSize = audioRequests.size();
                    }
                    auto logSystem = context->getLogSystem();
                    logSystem->print(
                        string::Format("tl::timeline::Timeline {0}").arg(this),
                        string::Format(
                        "\n"
                        "    Path: {0}\n"
                        "    Video requests: {1}, {2} in-progress, {3} max\n"
                        "    Audio requests: {4}, {5} in-progress, {6} max\n"
                        "    Readers: {7}").
                        arg(path.get()).
                        arg(videoRequestsSize).
                        arg(videoRequestsInProgress.size()).
                        arg(options.videoRequestCount).
                        arg(audioRequestsSize).
                        arg(audioRequestsInProgress.size()).
                        arg(options.audioRequestCount).
                        arg(readers.size()));
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
                std::unique_lock<std::mutex> lock(mutex);
                requestCV.wait_for(
                    lock,
                    options.requestTimeout,
                    [this]
                    {
                        return !videoRequests.empty() ||
                            !videoRequestsInProgress.empty() ||
                            !audioRequests.empty() ||
                            !audioRequestsInProgress.empty();
                    });
                while (!videoRequests.empty() &&
                    (videoRequestsInProgress.size() + newVideoRequests.size()) < options.videoRequestCount)
                {
                    newVideoRequests.push_back(videoRequests.front());
                    videoRequests.pop_front();
                }
                while (!audioRequests.empty() &&
                    (audioRequestsInProgress.size() + newAudioRequests.size()) < options.audioRequestCount)
                {
                    newAudioRequests.push_back(audioRequests.front());
                    audioRequests.pop_front();
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
                                const auto time = request->time - globalStartTime;
                                otio::ErrorStatus errorStatus;
                                const auto range = otioItem->trimmed_range_in_parent(&errorStatus);
                                if (range.has_value() && range.value().contains(time))
                                {
                                    VideoLayerData videoData;
                                    if (auto otioClip = dynamic_cast<const otio::Clip*>(otioItem))
                                    {
                                        if (readers.find(otioClip) == readers.end())
                                        {
                                            auto reader = createReader(otioTrack, otioClip, options.ioOptions);
                                            if (reader.read)
                                            {
                                                readers[otioClip] = reader;
                                            }
                                        }
                                        videoData.image = readVideo(otioTrack, otioClip, time, request->videoLayer);
                                    }
                                    const auto neighbors = otioTrack->neighbors_of(otioItem, &errorStatus);
                                    if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.second.value))
                                    {
                                        if (time > range.value().end_time_inclusive() - otioTransition->in_offset())
                                        {
                                            videoData.transition = toTransition(otioTransition->transition_type());
                                            videoData.transitionValue = transitionValue(
                                                time.value(),
                                                range.value().end_time_inclusive().value() - otioTransition->in_offset().value(),
                                                range.value().end_time_inclusive().value() + otioTransition->out_offset().value() + 1.0);
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.second.value))
                                            {
                                                if (readers.find(otioClipB) == readers.end())
                                                {
                                                    auto reader = createReader(otioTrack, otioClipB, options.ioOptions);
                                                    if (reader.read)
                                                    {
                                                        readers[otioClipB] = reader;
                                                    }
                                                }
                                                videoData.imageB = readVideo(otioTrack, otioClipB, time, request->videoLayer);
                                            }
                                        }
                                    }
                                    if (auto otioTransition = dynamic_cast<otio::Transition*>(neighbors.first.value))
                                    {
                                        if (time < range.value().start_time() + otioTransition->out_offset())
                                        {
                                            std::swap(videoData.image, videoData.imageB);
                                            videoData.transition = toTransition(otioTransition->transition_type());
                                            videoData.transitionValue = transitionValue(
                                                time.value(),
                                                range.value().start_time().value() - otioTransition->in_offset().value() - 1.0,
                                                range.value().start_time().value() + otioTransition->out_offset().value());
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<otio::Clip*>(transitionNeighbors.first.value))
                                            {
                                                if (readers.find(otioClipB) == readers.end())
                                                {
                                                    auto reader = createReader(otioTrack, otioClipB, options.ioOptions);
                                                    if (reader.read)
                                                    {
                                                        readers[otioClipB] = reader;
                                                    }
                                                }
                                                videoData.image = readVideo(otioTrack, otioClipB, time, request->videoLayer);
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

                videoRequestsInProgress.push_back(request);
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
                                const otime::RationalTime time = time::floor(
                                    otime::RationalTime(request->seconds, 1.0) - globalStartTime.rescaled_to(1.0));
                                const otime::TimeRange timeRange = otime::TimeRange(time, otime::RationalTime(1.0, 1.0));
                                otio::ErrorStatus errorStatus;
                                const auto range = otioItem->trimmed_range_in_parent(&errorStatus);
                                if (range.has_value() && range.value().intersects(timeRange))
                                {
                                    AudioLayerData audioData;
                                    if (auto otioClip = dynamic_cast<const otio::Clip*>(otioItem))
                                    {
                                        if (readers.find(otioClip) == readers.end())
                                        {
                                            auto reader = createReader(otioTrack, otioClip, options.ioOptions);
                                            if (reader.read)
                                            {
                                                readers[otioClip] = reader;
                                            }
                                        }
                                        audioData.audio = readAudio(otioTrack, otioClip, timeRange);
                                    }
                                    request->layerData.push_back(std::move(audioData));
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                audioRequestsInProgress.push_back(request);
            }

            // Check for finished video requests.
            auto videoRequestIt = videoRequestsInProgress.begin();
            while (videoRequestIt != videoRequestsInProgress.end())
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
                    videoRequestIt = videoRequestsInProgress.erase(videoRequestIt);
                    continue;
                }
                ++videoRequestIt;
            }

            // Check for finished audio requests.
            auto audioRequestIt = audioRequestsInProgress.begin();
            while (audioRequestIt != audioRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*audioRequestIt)->layerData)
                {
                    if (i.audio.valid())
                    {
                        valid &= i.audio.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    }
                    if (i.audioB.valid())
                    {
                        valid &= i.audioB.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
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
                                layer.audio = j.audio.get().audio;
                            }
                            data.layers.push_back(layer);
                        }
                    }
                    catch (const std::exception&)
                    {
                        //! \todo How should this be handled?
                    }
                    (*audioRequestIt)->promise.set_value(data);
                    audioRequestIt = audioRequestsInProgress.erase(audioRequestIt);
                    continue;
                }
                ++audioRequestIt;
            }
        }

        Timeline::Private::Reader Timeline::Private::createReader(
            const otio::Track* track,
            const otio::Clip* clip,
            const io::Options& ioOptions)
        {
            Reader out;
            if (auto context = this->context.lock())
            {
                const auto path = getPath(clip->media_reference());
                const auto memoryRead = getMemoryRead(clip->media_reference());
                io::Options options = ioOptions;
                options["SequenceIO/DefaultSpeed"] = string::Format("{0}").arg(duration.rate());
                const auto ioSystem = context->getSystem<io::System>();
                auto read = ioSystem->read(path, memoryRead, options);
                io::Info info;
                if (read)
                {
                    info = read->getInfo().get();
                }
                if (read)
                {
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
                        arg(!info.video.empty() ? info.video[0].size : imaging::Size()).
                        arg(!info.video.empty() ? info.video[0].pixelType : imaging::PixelType::None).
                        arg(info.videoTime).
                        arg(info.audio.channelCount).
                        arg(info.audio.dataType).
                        arg(info.audio.sampleRate).
                        arg(info.audioTime));

                    // Get the clip start and end time taking transitions into account.
                    otio::ErrorStatus errorStatus;
                    const auto range = clip->trimmed_range(&errorStatus);
                    otime::RationalTime startTime = range.start_time();
                    auto endTime = startTime + range.duration();
                    const auto neighbors = track->neighbors_of(clip, &errorStatus);
                    if (auto transition = dynamic_cast<const otio::Transition*>(neighbors.first.value))
                    {
                        startTime -= transition->in_offset();
                    }
                    if (auto transition = dynamic_cast<const otio::Transition*>(neighbors.second.value))
                    {
                        endTime += transition->out_offset();
                    }

                    out.read = read;
                    out.info = info;
                    const auto ancestor = dynamic_cast<const otio::Item*>(getRoot(clip));
                    out.range = clip->transformed_time_range(
                        otime::TimeRange::range_from_start_end_time(globalStartTime + startTime, globalStartTime + endTime),
                        ancestor,
                        &errorStatus);
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
            if (auto context = this->context.lock())
            {
                const auto j = readers.find(clip);
                if (j != readers.end())
                {
                    otio::ErrorStatus errorStatus;
                    const auto clipTime = track->transformed_time(time, clip, &errorStatus);
                    const auto readTime = time::floor(clipTime.rescaled_to(j->second.info.videoTime.duration().rate()));
                    out = j->second.read->readVideo(readTime, videoLayer);
                }
            }
            return out;
        }

        std::future<io::AudioData> Timeline::Private::readAudio(
            const otio::Track* track,
            const otio::Clip* clip,
            const otime::TimeRange& timeRange)
        {
            std::future<io::AudioData> out;
            if (auto context = this->context.lock())
            {
                const auto j = readers.find(clip);
                if (j != readers.end())
                {
                    otio::ErrorStatus errorStatus;
                    const auto clipRange = track->transformed_time_range(timeRange, clip, &errorStatus);
                    const auto readRange = otime::TimeRange(
                        time::floor(clipRange.start_time().rescaled_to(ioInfo.audio.sampleRate)),
                        time::ceil(clipRange.duration().rescaled_to(ioInfo.audio.sampleRate)));
                    out = j->second.read->readAudio(readRange);
                }
            }
            return out;
        }

        void Timeline::Private::stopReaders()
        {
            auto i = readers.begin();
            while (i != readers.end())
            {
                bool del = true;
                for (const auto& activeRange : activeRanges)
                {
                    if (i->second.range.intersects(activeRange))
                    {
                        del = false;
                        break;
                    }
                }
                if (del && !i->second.read->hasRequests())
                {
                    if (auto context = this->context.lock())
                    {
                        context->log("tl::timeline::Timeline", path.get() + ": Stop: " + i->second.read->getPath().get());
                    }
                    auto read = i->second.read;
                    read->stop();
                    stoppedReaders.push_back(read);
                    i = readers.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        void Timeline::Private::delReaders()
        {
            auto i = stoppedReaders.begin();
            while (i != stoppedReaders.end())
            {
                if ((*i)->hasStopped())
                {
                    if (auto context = this->context.lock())
                    {
                        context->log("tl::timeline::Timeline", path.get() + ": Delete: " + (*i)->getPath().get());
                    }
                    i = stoppedReaders.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }
}
