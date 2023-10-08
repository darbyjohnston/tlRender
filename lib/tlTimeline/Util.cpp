// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/Util.h>

#include <tlTimeline/MemoryReference.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

namespace tl
{
    namespace timeline
    {
        std::vector<std::string> getExtensions(
            int types,
            const std::shared_ptr<system::Context>& context)
        {
            std::vector<std::string> out;
            //! \todo Get extensions for the Python adapters?
            if (types & static_cast<int>(io::FileType::Movie))
            {
                out.push_back(".otio");
                out.push_back(".otioz");
            }
            if (auto ioSystem = context->getSystem<io::System>())
            {
                for (const auto& plugin : ioSystem->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions(types);
                    out.insert(out.end(), extensions.begin(), extensions.end());
                }
            }
            return out;
        }

        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime> frames)
        {
            std::vector<otime::TimeRange> out;
            if (!frames.empty())
            {
                std::sort(frames.begin(), frames.end());
                auto i = frames.begin();
                auto j = i;
                do
                {
                    auto k = j + 1;
                    if (k != frames.end() && (*k - *j).value() > 1)
                    {
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else if (k == frames.end())
                    {
                        out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else
                    {
                        ++j;
                    }
                } while (j != frames.end());
            }
            return out;
        }

        otime::RationalTime loop(
            const otime::RationalTime& value,
            const otime::TimeRange& range,
            bool* looped)
        {
            auto out = value;
            if (out < range.start_time())
            {
                if (looped)
                {
                    *looped = true;
                }
                out = range.end_time_inclusive();
            }
            else if (out > range.end_time_inclusive())
            {
                if (looped)
                {
                    *looped = true;
                }
                out = range.start_time();
            }
            return out;
        }

        std::vector<otime::TimeRange> loopCache(
            const otime::TimeRange& value,
            const otime::TimeRange& range,
            CacheDirection direction)
        {
            std::vector<otime::TimeRange> out;
            const otime::RationalTime min = std::min(value.duration(), range.duration());
            switch (direction)
            {
            case CacheDirection::Forward:
                if (value.start_time() < range.start_time())
                {
                    const otime::TimeRange a(range.start_time(), min);
                    TLRENDER_ASSERT(a.duration() == min);
                    out.push_back(a);
                }
                else if (value.start_time() > range.end_time_inclusive())
                {
                    const otime::TimeRange a(range.end_time_exclusive() - min, min);
                    TLRENDER_ASSERT(a.duration() == min);
                    out.push_back(a);
                }
                else if (value.end_time_inclusive() > range.end_time_exclusive())
                {
                    const otime::TimeRange clamped(value.start_time(), min);
                    const otime::TimeRange a = otime::TimeRange::range_from_start_end_time_inclusive(
                        clamped.start_time(),
                        range.end_time_inclusive());
                    const otime::TimeRange b = otime::TimeRange(
                        range.start_time(),
                        clamped.duration() - a.duration());
                    TLRENDER_ASSERT(a.duration() + b.duration() == min);
                    if (a.duration().value() > 0.0)
                    {
                        out.push_back(a);
                    }
                    if (b.duration().value() > 0.0)
                    {
                        out.push_back(b);
                    }
                }
                else
                {
                    out.push_back(value);
                }
                break;
            case CacheDirection::Reverse:
                if (value.end_time_inclusive() > range.end_time_inclusive())
                {
                    const otime::TimeRange a(range.end_time_exclusive() - min, min);
                    out.push_back(a);
                    TLRENDER_ASSERT(a.duration() == min);
                }
                else if (value.end_time_inclusive() < range.start_time())
                {
                    const otime::TimeRange a(range.start_time(), min);
                    out.push_back(a);
                    TLRENDER_ASSERT(a.duration() == min);
                }
                else if (value.start_time() < range.start_time())
                {
                    const otime::TimeRange clamped = otime::TimeRange::range_from_start_end_time_inclusive(
                        value.end_time_exclusive() - min,
                        value.end_time_inclusive());
                    const otime::TimeRange a = otime::TimeRange::range_from_start_end_time_inclusive(
                        range.start_time(),
                        clamped.end_time_inclusive());
                    const otime::TimeRange b = otime::TimeRange::range_from_start_end_time_inclusive(
                        range.end_time_exclusive() - (clamped.duration() - a.duration()),
                        range.end_time_inclusive());
                    TLRENDER_ASSERT(a.duration() + b.duration() == min);
                    if (a.duration().value() > 0.0)
                    {
                        out.push_back(a);
                    }
                    if (b.duration().value() > 0.0)
                    {
                        out.push_back(b);
                    }
                }
                else
                {
                    out.push_back(value);
                }
                break;
            default: break;
            }
            return out;
        }

        const otio::Composable* getRoot(const otio::Composable* composable)
        {
            const otio::Composable* out = composable;
            for (; out->parent(); out = out->parent())
                ;
            return out;
        }

        otio::optional<otime::RationalTime> getDuration(
            const otio::Timeline* otioTimeline,
            const std::string& kind)
        {
            otio::optional<otime::RationalTime> out;
            otio::ErrorStatus errorStatus;
            for (auto track : otioTimeline->children_if<otio::Track>(&errorStatus))
            {
                if (kind == track->kind())
                {
                    const otime::RationalTime duration = track->duration(&errorStatus);
                    if (out.has_value())
                    {
                        out = std::max(out.value(), duration);
                    }
                    else
                    {
                        out = duration;
                    }
                }
            }
            return out;
        }

        otime::TimeRange getTimeRange(const otio::Timeline* otioTimeline)
        {
            otime::TimeRange out = time::invalidTimeRange;
            auto duration = timeline::getDuration(otioTimeline, otio::Track::Kind::video);
            if (!duration.has_value())
            {
                duration = timeline::getDuration(otioTimeline, otio::Track::Kind::audio);
            }
            if (duration.has_value())
            {
                const otime::RationalTime startTime = otioTimeline->global_start_time().has_value() ?
                    otioTimeline->global_start_time().value().rescaled_to(duration->rate()) :
                    otime::RationalTime(0, duration->rate());
                out = otime::TimeRange(startTime, duration.value());
            }
            return out;
        }

        std::vector<file::Path> getPaths(
            const file::Path& path,
            const file::PathOptions& pathOptions,
            const std::shared_ptr<system::Context>& context)
        {
            std::vector<file::Path> out;
            const auto fileInfo = file::FileInfo(path);
            switch (fileInfo.getType())
            {
            case file::Type::Directory:
            {
                auto ioSystem = context->getSystem<io::System>();
                file::ListOptions listOptions;
                listOptions.maxNumberDigits = pathOptions.maxNumberDigits;
                std::vector<file::FileInfo> list;
                file::list(path.get(), list, listOptions);
                for (const auto& fileInfo : list)
                {
                    const file::Path& path = fileInfo.getPath();
                    const std::string extension = string::toLower(path.getExtension());
                    switch (ioSystem->getFileType(extension))
                    {
                    case io::FileType::Sequence:
                    case io::FileType::Movie:
                    case io::FileType::Audio:
                        out.push_back(path);
                        break;
                    default:
                        //! \todo Get extensions for the Python adapters?
                        if (".otio" == extension ||
                            ".otioz" == extension)
                        {
                            out.push_back(path);
                        }
                        break;
                    }
                }
                break;
            }
            default:
                out.push_back(path);
                break;
            }
            return out;
        }

        namespace
        {
            const std::vector<std::string> fileURLPrefixes =
            {
                "file:////",
                "file:///",
                "file://"
            };
        }

        std::string removeFileURLPrefix(const std::string& value)
        {
            std::string out = value;
            for (const auto& prefix :fileURLPrefixes)
            {
                if (0 == out.compare(0, prefix.size(), prefix))
                {
                    out.replace(0, prefix.size(), "");
                    break;
                }
            }
            return out;
        }

        file::Path getPath(
            const std::string& url,
            const std::string& directory,
            const file::PathOptions& options)
        {
            file::Path out = file::Path(removeFileURLPrefix(url), options);
            if (!out.isAbsolute())
            {
                out = file::Path(directory, out.get(), options);
            }
            return out;
        }

        file::Path getPath(
            const otio::MediaReference* ref,
            const std::string& directory,
            file::PathOptions pathOptions)
        {
            std::string url;
            math::IntRange sequence;
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
                sequence = math::IntRange(
                    imageSequenceRef->start_frame(),
                    imageSequenceRef->end_frame());
            }
            else if (auto rawMemoryRef = dynamic_cast<const RawMemoryReference*>(ref))
            {
                url = rawMemoryRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (auto sharedMemoryRef = dynamic_cast<const SharedMemoryReference*>(ref))
            {
                url = sharedMemoryRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (auto rawMemorySequenceRef = dynamic_cast<const RawMemorySequenceReference*>(ref))
            {
                url = rawMemorySequenceRef->target_url();
            }
            else if (auto sharedMemorySequenceRef = dynamic_cast<const SharedMemorySequenceReference*>(ref))
            {
                url = sharedMemorySequenceRef->target_url();
            }
            file::Path out = timeline::getPath(url, directory, pathOptions);
            if (sequence.getMin() != sequence.getMax())
            {
                out.setSequence(sequence);
            }
            return out;
        }

        std::vector<file::MemoryRead> getMemoryRead(
            const otio::MediaReference* ref)
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

        void toMemoryReferences(
            otio::Timeline* otioTimeline,
            const std::string& directory,
            const file::PathOptions& pathOptions)
        {
            // Recursively iterate over all clips in the timeline.
            for (auto clip : otioTimeline->children_if<otio::Clip>())
            {
                if (auto externalReference =
                    dynamic_cast<otio::ExternalReference*>(clip->media_reference()))
                {
                    // Get the external reference path.
                    const auto path = timeline::getPath(externalReference->target_url(), directory, pathOptions);

                    // Read the external reference media into memory.
                    auto fileIO = file::FileIO::create(path.get(), file::Mode::Read);
                    const size_t size = fileIO->getSize();
                    auto memory = std::make_shared<timeline::MemoryReferenceData>();
                    memory->resize(size);
                    fileIO->read(memory->data(), size);

                    // Replace the external reference with a memory reference.
                    auto memoryReference = new timeline::SharedMemoryReference(
                        externalReference->target_url(),
                        memory,
                        clip->available_range(),
                        externalReference->metadata());
                    clip->set_media_reference(memoryReference);
                }
                else if (auto imageSequenceRefence =
                    dynamic_cast<otio::ImageSequenceReference*>(clip->media_reference()))
                {
                    // Get the image sequence reference path.
                    int padding = imageSequenceRefence->frame_zero_padding();
                    std::string number;
                    std::stringstream ss;
                    ss << imageSequenceRefence->target_url_base() <<
                        imageSequenceRefence->name_prefix() <<
                        std::setfill('0') << std::setw(padding) << imageSequenceRefence->start_frame() <<
                        imageSequenceRefence->name_suffix();
                    const auto path = timeline::getPath(ss.str(), directory, pathOptions);

                    // Read the image sequence reference media into memory.
                    std::vector<std::shared_ptr<timeline::MemoryReferenceData> > memoryList;
                    const auto range = clip->trimmed_range();
                    for (
                        int64_t frame = imageSequenceRefence->start_frame();
                        frame < imageSequenceRefence->start_frame() + range.duration().value();
                        ++frame)
                    {
                        const auto& fileName = path.get(frame);
                        auto fileIO = file::FileIO::create(fileName, file::Mode::Read);
                        const size_t size = fileIO->getSize();
                        auto memory = std::make_shared<timeline::MemoryReferenceData>();
                        memory->resize(size);
                        fileIO->read(memory->data(), size);
                        memoryList.push_back(memory);
                    }

                    // Replace the image sequence reference with a memory
                    // sequence reference.
                    auto memorySequenceReference = new timeline::SharedMemorySequenceReference(
                        path.get(),
                        memoryList,
                        clip->available_range(),
                        imageSequenceRefence->metadata());
                    clip->set_media_reference(memorySequenceReference);
                }
            }
        }

        otime::RationalTime toVideoMediaTime(
            const otime::RationalTime& time,
            const otio::Clip* clip,
            double rate)
        {
            otime::RationalTime clipTime;
            if (auto parent = clip->parent())
            {
                clipTime = parent->transformed_time(time, clip);
            }
            const auto mediaTime = time::round(clipTime.rescaled_to(rate));
            return mediaTime;
        }

        otime::TimeRange toAudioMediaTime(
            const otime::TimeRange& timeRange,
            const otio::Clip* clip,
            double sampleRate)
        {
            otime::TimeRange clipRange;
            if (auto parent = clip->parent())
            {
                clipRange = parent->transformed_time_range(timeRange, clip);
            }
            const otime::TimeRange mediaRange(
                time::round(clipRange.start_time().rescaled_to(sampleRate)),
                time::round(clipRange.duration().rescaled_to(sampleRate)));
            return mediaRange;
        }
    }
}
