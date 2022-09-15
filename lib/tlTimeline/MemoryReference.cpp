// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/MemoryReference.h>

namespace tl
{
    namespace timeline
    {
        MemoryReference::MemoryReference(
            std::string const& target_url,
            uint8_t* memory_ptr,
            size_t memory_size,
            otio::optional<otio::TimeRange> const& available_range,
            otio::AnyDictionary const& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory_ptr(memory_ptr),
            _memory_size(memory_size)
        {}

        MemoryReference::~MemoryReference()
        {}

        std::string MemoryReference::target_url() const noexcept
        {
            return _target_url;
        }

        void MemoryReference::set_target_url(std::string const& target_url)
        {
            _target_url = target_url;
        }

        uint8_t* MemoryReference::memory_ptr() const noexcept
        {
            return _memory_ptr;
        }

        size_t MemoryReference::memory_size() const noexcept
        {
            return _memory_size;
        }

        void MemoryReference::set_memory_ptr(uint8_t* value)
        {
            _memory_ptr = value;
        }

        void MemoryReference::set_memory_size(size_t value)
        {
            _memory_size = value;
        }
    }
}
