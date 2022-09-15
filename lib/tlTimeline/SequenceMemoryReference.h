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
                std::string const& target_url = std::string(),
                std::vector<uint8_t*> const& memory_ptrs = std::vector<uint8_t*>(),
                std::vector<size_t> const& memory_sizes = std::vector<size_t>(),
                otio::optional<otio::TimeRange> const& available_range = otio::nullopt,
                otio::AnyDictionary const& metadata = otio::AnyDictionary());

            std::string target_url() const noexcept;

            void set_target_url(std::string const&);

            std::vector<uint8_t*> const& memory_ptrs() const noexcept;
            std::vector<size_t> const& memory_sizes() const noexcept;

            void set_memory_ptrs(std::vector<uint8_t*> const&);
            void set_memory_sizes(std::vector<size_t> const&);

        protected:
            virtual ~SequenceMemoryReference();

        private:
            std::string _target_url;
            std::vector<uint8_t*> _memory_ptrs;
            std::vector<size_t> _memory_sizes;
        };
    }
}
