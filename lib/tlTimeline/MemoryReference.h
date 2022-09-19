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
                const std::string& target_url = std::string(),
                const uint8_t* memory_ptr = nullptr,
                size_t memory_size = 0,
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const uint8_t* memory_ptr() const noexcept;
            size_t memory_size() const noexcept;

            void set_memory_ptr(const uint8_t*);
            void set_memory_size(size_t);

        protected:
            virtual ~MemoryReference();

        private:
            std::string _target_url;
            const uint8_t* _memory_ptr = nullptr;
            size_t _memory_size = 0;
        };
    }
}
