// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include <opentimelineio/mediaReference.h>

namespace tl
{
    namespace timeline
    {
        class MemoryReference final : public otio::MediaReference
        {
        public:
            MemoryReference(
                std::string const& target_url = std::string(),
                uint8_t* memory_ptr = nullptr,
                size_t memory_size = 0,
                otio::optional<otio::TimeRange> const& available_range = otio::nullopt,
                otio::AnyDictionary const& metadata = otio::AnyDictionary());

            std::string target_url() const noexcept;

            void set_target_url(std::string const&);

            uint8_t* memory_ptr() const noexcept;
            size_t memory_size() const noexcept;

            void set_memory_ptr(uint8_t*);
            void set_memory_size(size_t);

        protected:
            virtual ~MemoryReference();

        private:
            std::string _target_url;
            uint8_t* _memory_ptr = nullptr;
            size_t _memory_size = 0;
        };
    }
}
