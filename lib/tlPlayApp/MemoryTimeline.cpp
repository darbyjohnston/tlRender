// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/MemoryTimeline.h>

#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

namespace tl
{
    namespace play
    {
        InMemoryReference::InMemoryReference(
            const std::string& target_url,
            const io::MemoryRead& memory,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            timeline::MemoryReference(target_url, memory, available_range, metadata)
        {}

        void InMemoryReference::load(
            const std::string& directory,
            const file::PathOptions& pathOptions)
        {
            const auto path = timeline::getPath(_target_url, directory, pathOptions);
            auto fileIO = file::FileIO::create(path.get(), file::Mode::Read);
            const size_t size = fileIO->getSize();
            _data.resize(size);
            fileIO->read(&_data[0], size);
            _memory.p = &_data[0];
            _memory.size = size;
        }

        InMemorySequenceReference::InMemorySequenceReference(
            const std::string& target_url,
            const std::vector<io::MemoryRead>& memory,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            timeline::MemorySequenceReference(target_url, memory, available_range, metadata)
        {}

        void InMemorySequenceReference::load(
            const std::string& directory,
            const file::PathOptions& pathOptions)
        {
            const auto& range = available_range();
            if (range.has_value())
            {
                const auto path = timeline::getPath(_target_url, directory, pathOptions);
                for (
                    int64_t frame = range->start_time().value();
                    frame < range->duration().value();
                    ++frame)
                {
                    const auto& fileName = path.get(frame);
                    auto fileIO = file::FileIO::create(fileName, file::Mode::Read);
                    const size_t size = fileIO->getSize();
                    _data.push_back(std::vector<uint8_t>());
                    _data.back().resize(size);
                    fileIO->read(&_data.back()[0], size);
                    _memory.push_back(io::MemoryRead(&_data.back()[0], size));
                }
            }
        }

        void loadMemory(
            otio::Timeline* otioTimeline,
            const std::string& directory,
            const file::PathOptions& pathOptions)
        {
            for (auto clip : otioTimeline->children_if<otio::Clip>())
            {
                if (auto externalReference = dynamic_cast<otio::ExternalReference*>(clip->media_reference()))
                {
                    auto memoryReference = new InMemoryReference(
                        externalReference->target_url(),
                        io::MemoryRead(),
                        externalReference->available_range(),
                        externalReference->metadata());
                    clip->set_media_reference(memoryReference);
                    memoryReference->load(directory, pathOptions);
                }
                else if (auto imageSequenceRefence = dynamic_cast<otio::ImageSequenceReference*>(clip->media_reference()))
                {
                    int padding = imageSequenceRefence->frame_zero_padding();
                    std::string number;
                    std::stringstream ss;
                    ss << std::setfill('0') << std::setw(padding) << imageSequenceRefence->start_frame();
                    ss >> number;
                    const file::Path path(
                        imageSequenceRefence->target_url_base(),
                        imageSequenceRefence->name_prefix(),
                        number,
                        static_cast<uint8_t>(padding),
                        imageSequenceRefence->name_suffix());
                    auto memoryReference = new InMemoryReference(
                        path.get(),
                        io::MemoryRead(),
                        imageSequenceRefence->available_range(),
                        imageSequenceRefence->metadata());
                    clip->set_media_reference(memoryReference);
                    memoryReference->load(directory, pathOptions);
                }
            }
        }
    }
}
