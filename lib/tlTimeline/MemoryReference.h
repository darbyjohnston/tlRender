// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>

#include <memory>

namespace tl
{
    namespace timeline
    {
        //! Memory reference data.
        typedef std::vector<uint8_t> MemoryReferenceData;

        //! Read references from memory.
        class MemoryReference : public otio::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "MemoryReference";
                static int constexpr version = 1;
            };

            /*MemoryReference(
                const std::string& target_url = std::string(),
                const uint8_t* memory_ptr = nullptr,
                size_t memory_size = 0,
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());*/
            MemoryReference(
                const std::string& target_url = std::string(),
                const std::shared_ptr<MemoryReferenceData>& memory_data = nullptr,
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const uint8_t* memory_ptr() const noexcept;

            size_t memory_size() const noexcept;

            void set_memory(const uint8_t* memory_ptr, size_t memory_size);

            void set_memory_data(const std::shared_ptr<MemoryReferenceData>&);

        protected:
            virtual ~MemoryReference() override;

            std::string _target_url;
            const uint8_t* _memory_ptr = nullptr;
            size_t _memory_size = 0;
            std::shared_ptr<MemoryReferenceData> _memory_data;
        };

        //! Read sequence references from memory.
        class MemorySequenceReference : public otio::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "MemorySequenceReference";
                static int constexpr version = 1;
            };

            /*MemorySequenceReference(
                const std::string& target_url = std::string(),
                const std::vector<const uint8_t*>& memory_ptrs = {},
                const std::vector<size_t> memory_sizes = {},
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());*/
            MemorySequenceReference(
                const std::string& target_url = std::string(),
                const std::vector<std::shared_ptr<MemoryReferenceData> >& memory_data = {},
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<const uint8_t*>& memory_ptrs() const noexcept;

            const std::vector<size_t>& memory_sizes() const noexcept;

            void set_memory(
                const std::vector<const uint8_t*>& memory_ptrs,
                const std::vector<size_t>& memory_sizes);

            void set_memory_data(
                const std::vector<std::shared_ptr<MemoryReferenceData> >& memory_data);

        protected:
            virtual ~MemorySequenceReference() override;

            std::string _target_url;
            std::vector<const uint8_t*> _memory_ptrs;
            std::vector<size_t> _memory_sizes;
            std::vector<std::shared_ptr<MemoryReferenceData> > _memory_data;
        };
    }
}
