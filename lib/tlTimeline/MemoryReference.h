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
        //! Read references from memory.
        class MemoryReference : public otio::MediaReference
        {
        public:
            MemoryReference(
                const std::string& target_url = std::string(),
                const io::MemoryRead& memory = io::MemoryRead(),
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const io::MemoryRead& memory() const noexcept;

            void set_memory(const io::MemoryRead&);

        protected:
            virtual ~MemoryReference() override;

            std::string _target_url;
            io::MemoryRead _memory;
        };

        //! Read sequence references from memory.
        class MemorySequenceReference : public otio::MediaReference
        {
        public:
            MemorySequenceReference(
                const std::string& target_url = std::string(),
                const std::vector<io::MemoryRead>& memory = std::vector<io::MemoryRead>(),
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<io::MemoryRead>& memory() const noexcept;

            void set_memory(const std::vector<io::MemoryRead>&);

        protected:
            virtual ~MemorySequenceReference() override;

            std::string _target_url;
            std::vector<io::MemoryRead> _memory;
        };
    }
}
