// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtPlayApp/MemoryTimeline.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

namespace tl
{
    namespace qtplay
    {
        void createMemoryTimeline(
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
    }
}
