// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Read references from in-memory.
        class MemoryReference final : public otio::MediaReference
        {
        public:
            MemoryReference(
                const std::string& target_url = std::string(),
                const std::vector<io::MemoryRead>& memory = std::vector<io::MemoryRead>(),
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<io::MemoryRead>& memory() const noexcept;

            void set_memory(const std::vector<io::MemoryRead>&);

        protected:
            virtual ~MemoryReference();

        private:
            std::string _target_url;
            std::vector<io::MemoryRead> _memory;
        };
    }
}
