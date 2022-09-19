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
        class SequenceMemoryReference final : public otio::MediaReference
        {
        public:
            SequenceMemoryReference(
                const std::string& target_url = std::string(),
                const std::vector<const uint8_t*>& memory_ptrs = std::vector<const uint8_t*>(),
                const std::vector<size_t>& memory_sizes = std::vector<size_t>(),
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<const uint8_t*>& memory_ptrs() const noexcept;
            const std::vector<size_t>& memory_sizes() const noexcept;

            void set_memory_ptrs(const std::vector<const uint8_t*>&);
            void set_memory_sizes(const std::vector<size_t>&);

        protected:
            virtual ~SequenceMemoryReference();

        private:
            std::string _target_url;
            std::vector<const uint8_t*> _memory_ptrs;
            std::vector<size_t> _memory_sizes;
        };
    }
}
