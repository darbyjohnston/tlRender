// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Timeline.h>

#include <tlCore/FileIO.h>

namespace tl
{
    namespace play
    {
        class InMemoryReference : public timeline::MemoryReference
        {
        public:
            InMemoryReference(
                const std::string& target_url = std::string(),
                const io::MemoryRead& memory = io::MemoryRead(),
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            void load(
                const std::string& directory,
                const file::PathOptions&);

        private:
            std::vector<uint8_t> _data;
        };

        class InMemorySequenceReference : public timeline::MemorySequenceReference
        {
        public:
            InMemorySequenceReference(
                const std::string& target_url = std::string(),
                const std::vector<io::MemoryRead>& memory = std::vector<io::MemoryRead>(),
                const otio::optional<otio::TimeRange>& available_range = otio::nullopt,
                const otio::AnyDictionary& metadata = otio::AnyDictionary());

            void load(
                const std::string& directory,
                const file::PathOptions&);

        private:
            std::vector<std::vector<uint8_t> > _data;
        };

        void loadMemory(
            otio::Timeline*,
            const std::string& directory,
            const file::PathOptions&);
    }
}
