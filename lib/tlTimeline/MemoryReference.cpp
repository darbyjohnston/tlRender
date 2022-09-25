// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/MemoryReference.h>

namespace tl
{
    namespace timeline
    {
        MemoryReference::MemoryReference(
            const std::string& target_url,
            const io::MemoryRead& memory,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory(memory)
        {}

        MemoryReference::~MemoryReference()
        {}

        const std::string& MemoryReference::target_url() const noexcept
        {
            return _target_url;
        }

        void MemoryReference::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const io::MemoryRead& MemoryReference::memory() const noexcept
        {
            return _memory;
        }

        void MemoryReference::set_memory(const io::MemoryRead& value)
        {
            _memory = value;
        }

        MemorySequenceReference::MemorySequenceReference(
            const std::string& target_url,
            const std::vector<io::MemoryRead>& memory,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory(memory)
        {}

        MemorySequenceReference::~MemorySequenceReference()
        {}

        const std::string& MemorySequenceReference::target_url() const noexcept
        {
            return _target_url;
        }

        void MemorySequenceReference::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const std::vector<io::MemoryRead>& MemorySequenceReference::memory() const noexcept
        {
            return _memory;
        }

        void MemorySequenceReference::set_memory(const std::vector<io::MemoryRead>& value)
        {
            _memory = value;
        }
    }
}
