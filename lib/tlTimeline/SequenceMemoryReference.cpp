// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/SequenceMemoryReference.h>

namespace tl
{
    namespace timeline
    {
        SequenceMemoryReference::SequenceMemoryReference(
            std::string const& target_url,
            std::vector<uint8_t*> const& memory_ptrs,
            std::vector<size_t> const& memory_sizes,
            otio::optional<otio::TimeRange> const& available_range,
            otio::AnyDictionary const& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory_ptrs(memory_ptrs),
            _memory_sizes(memory_sizes)
        {}

        SequenceMemoryReference::~SequenceMemoryReference()
        {}

        std::string SequenceMemoryReference::target_url() const noexcept
        {
            return _target_url;
        }

        void SequenceMemoryReference::set_target_url(std::string const& target_url)
        {
            _target_url = target_url;
        }

        std::vector<uint8_t*> const& SequenceMemoryReference::memory_ptrs() const noexcept
        {
            return _memory_ptrs;
        }

        std::vector<size_t> const& SequenceMemoryReference::memory_sizes() const noexcept
        {
            return _memory_sizes;
        }

        void SequenceMemoryReference::set_memory_ptrs(std::vector<uint8_t*> const& value)
        {
            _memory_ptrs = value;
        }

        void SequenceMemoryReference::set_memory_sizes(std::vector<size_t> const& value)
        {
            _memory_sizes = value;
        }
    }
}
