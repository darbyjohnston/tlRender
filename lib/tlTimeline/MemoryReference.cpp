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
            const uint8_t* memory_ptr,
            size_t memory_size,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url)
        {
            set_memory(memory_ptr, memory_size);
        }

        MemoryReference::MemoryReference(
            const std::string & target_url,
            const std::shared_ptr<MemoryReferenceData>& memory_data,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url)
        {
            set_memory_data(memory_data);
        }

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

        const uint8_t* MemoryReference::memory_ptr() const noexcept
        {
            return _memory_ptr;
        }

        size_t MemoryReference::memory_size() const noexcept
        {
            return _memory_size;
        }

        void MemoryReference::set_memory(const uint8_t* memory_ptr, size_t memory_size)
        {
            _memory_ptr = memory_ptr;
            _memory_size = memory_size;
            _memory_data.reset();
        }

        void MemoryReference::set_memory_data(const std::shared_ptr<MemoryReferenceData>& memory_data)
        {
            _memory_ptr = memory_data->data();
            _memory_size = memory_data->size();
            _memory_data = memory_data;
        }

        MemorySequenceReference::MemorySequenceReference(
            const std::string& target_url,
            const std::vector<const uint8_t*>& memory_ptrs,
            const std::vector<size_t> memory_sizes,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url)
        {
            set_memory(memory_ptrs, memory_sizes);
        }

        MemorySequenceReference::MemorySequenceReference(
            const std::string& target_url,
            const std::vector<std::shared_ptr<MemoryReferenceData> >& memory_data,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url)
        {
            set_memory_data(memory_data);
        }

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

        const std::vector<const uint8_t*>& MemorySequenceReference::memory_ptrs() const noexcept
        {
            return _memory_ptrs;
        }

        const std::vector<size_t>& MemorySequenceReference::memory_sizes() const noexcept
        {
            return _memory_sizes;
        }

        void MemorySequenceReference::set_memory(
            const std::vector<const uint8_t*>& memory_ptrs,
            const std::vector<size_t>& memory_sizes)
        {
            _memory_ptrs = memory_ptrs;
            _memory_sizes = memory_sizes;
            _memory_data.clear();
        }

        void MemorySequenceReference::set_memory_data(
            const std::vector<std::shared_ptr<MemoryReferenceData> >& memory_data)
        {
            _memory_ptrs.clear();
            _memory_sizes.clear();
            for (const auto& i : memory_data)
            {
                _memory_ptrs.push_back(i->data());
                _memory_sizes.push_back(i->size());
            }
            _memory_data = memory_data;
        }
    }
}
