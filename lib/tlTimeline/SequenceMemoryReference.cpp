// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/SequenceMemoryReference.h>

namespace tl
{
    namespace timeline
    {
        SequenceMemoryReference::SequenceMemoryReference(
            const std::string& target_url,
            const std::vector<const uint8_t*>& memory_ptrs,
            const std::vector<size_t>& memory_sizes,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory_ptrs(memory_ptrs),
            _memory_sizes(memory_sizes)
        {}

        SequenceMemoryReference::~SequenceMemoryReference()
        {}

        const std::string& SequenceMemoryReference::target_url() const noexcept
        {
            return _target_url;
        }

        void SequenceMemoryReference::set_target_url(const std::string& target_url)
        {
            _target_url = target_url;
        }

        const std::vector<const uint8_t*>& SequenceMemoryReference::memory_ptrs() const noexcept
        {
            return _memory_ptrs;
        }

        const std::vector<size_t>& SequenceMemoryReference::memory_sizes() const noexcept
        {
            return _memory_sizes;
        }

        void SequenceMemoryReference::set_memory_ptrs(const std::vector<const uint8_t*>& value)
        {
            _memory_ptrs = value;
        }

        void SequenceMemoryReference::set_memory_sizes(const std::vector<size_t>& value)
        {
            _memory_sizes = value;
        }
    }
}
