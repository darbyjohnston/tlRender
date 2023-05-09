// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/Util.h>

#include <tlTimeline/MemoryReference.h>

#include <tlIO/IOSystem.h>
#include <tlIO/Util.h>

#include <tlCore/Context.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/typeRegistry.h>

namespace tl
{
    namespace timeline
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            io::init(context);

            const std::vector<std::pair<std::string, bool> > registerTypes
            {
                {
                    "RawMemoryReference",
                    otio::TypeRegistry::instance().register_type<tl::timeline::RawMemoryReference>()
                },
                {
                    "SharedMemoryReference",
                    otio::TypeRegistry::instance().register_type<tl::timeline::SharedMemoryReference>()
                },
                {
                    "RawMemorySequenceReference",
                    otio::TypeRegistry::instance().register_type<tl::timeline::RawMemorySequenceReference>()
                },
                {
                    "SharedMemorySequenceReference",
                    otio::TypeRegistry::instance().register_type<tl::timeline::SharedMemorySequenceReference>()
                },
            };
            for (const auto& t : registerTypes)
            {
                context->log(
                    "tl::timeline::init",
                    string::Format("register type {0}: {1}").
                        arg(t.first).
                        arg(t.second));
            }
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

        std::vector<otime::TimeRange> loop(
            const otime::TimeRange& value,
            const otime::TimeRange& range)
        {
            std::vector<otime::TimeRange> out;
            if (value.duration() >= range.duration())
            {
                out.push_back(range);
            }
            else if (value.start_time() >= range.start_time() &&
                value.end_time_inclusive() <= range.end_time_inclusive())
            {
                out.push_back(value);
            }
            else if (value.start_time() < range.start_time())
            {
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    range.end_time_exclusive() - (range.start_time() - value.start_time()),
                    range.end_time_inclusive()));
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    range.start_time(),
                    value.end_time_inclusive()));
            }
            else if (value.end_time_inclusive() > range.end_time_inclusive())
            {
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    value.start_time(),
                    range.end_time_inclusive()));
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    range.start_time(),
                    range.start_time() + (value.end_time_inclusive() - range.end_time_exclusive())));
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
            const std::string& fileName,
            const file::PathOptions& pathOptions,
            const std::shared_ptr<system::Context>& context)
        {
            std::vector<file::Path> out;
            const auto path = file::Path(fileName);
            const auto fileInfo = file::FileInfo(path);
            switch (fileInfo.getType())
            {
            case file::Type::Directory:
            {
                auto ioSystem = context->getSystem<io::System>();
                for (const auto& fileInfo : file::dirList(fileName, pathOptions))
                {
                    const file::Path& path = fileInfo.getPath();
                    const std::string extension = string::toLower(path.getExtension());
                    switch (ioSystem->getFileType(extension))
                    {
                    case io::FileType::Sequence:
                    {
                        if (out.empty() || path.getNumber().empty())
                        {
                            out.push_back(path);
                        }
                        else
                        {
                            bool exists = false;
                            for (const auto& i : out)
                            {
                                if (i.getDirectory() == path.getDirectory() &&
                                    i.getBaseName() == path.getBaseName() &&
                                    !i.getNumber().empty() &&
                                    i.getPadding() == path.getPadding())
                                {
                                    exists = true;
                                    break;
                                }
                            }
                            if (!exists)
                            {
                                out.push_back(path);
                            }
                        }
                        break;
                    }
                    case io::FileType::Movie:
                    case io::FileType::Audio:
                        out.push_back(path);
                        break;
                    default:
                        //! \todo Get extensions for the Python adapters.
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
            const std::string fileURLPrefix = "file://";
        }

        std::string removeFileURLPrefix(const std::string& value)
        {
            std::string out = value;
            if (0 == out.compare(0, fileURLPrefix.size(), fileURLPrefix))
            {
                out.replace(0, fileURLPrefix.size(), "");
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
            return timeline::getPath(url, directory, pathOptions);
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

        otime::RationalTime toVideoMediaTime(
            const otime::RationalTime& time,
            const otio::Track* track,
            const otio::Clip* clip,
            const io::Info& ioInfo)
        {
            otime::RationalTime clipTime = track->transformed_time(time, clip);
            if (auto externalReference =
                dynamic_cast<const otio::ExternalReference*>(clip->media_reference()))
            {
                // If the available range start time is greater than the
                // video end time we assume the media is missing timecode
                // and adjust accordingly.
                const auto availableRangeOpt = externalReference->available_range();
                if (availableRangeOpt.has_value() &&
                    availableRangeOpt->start_time() > ioInfo.videoTime.end_time_inclusive())
                {
                    clipTime -= availableRangeOpt->start_time();
                }
            }
            const auto mediaTime = time::round(
                clipTime.rescaled_to(ioInfo.videoTime.duration().rate()));
            return mediaTime;
        }

        otime::TimeRange toAudioMediaTime(
            const otime::TimeRange& timeRange,
            const otio::Track* track,
            const otio::Clip* clip,
            const io::Info& ioInfo)
        {
            otime::TimeRange clipRange = track->transformed_time_range(timeRange, clip);
            if (auto externalReference = dynamic_cast<const otio::ExternalReference*>(clip->media_reference()))
            {
                // If the available range start time is greater than the
                // video end time we assume the media is missing timecode
                // and adjust accordingly.
                const auto availableRangeOpt = externalReference->available_range();
                if (availableRangeOpt.has_value() &&
                    availableRangeOpt->start_time() > ioInfo.audioTime.end_time_inclusive())
                { 
                    clipRange = otime::TimeRange(
                        clipRange.start_time() - availableRangeOpt->start_time(),
                        clipRange.duration());
                }
            }
            const otime::TimeRange mediaRange(
                time::floor(clipRange.start_time().rescaled_to(ioInfo.audio.sampleRate)),
                time::ceil(clipRange.duration().rescaled_to(ioInfo.audio.sampleRate)));
            return mediaRange;
        }
    }
}
