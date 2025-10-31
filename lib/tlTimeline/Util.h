// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTimeline/Player.h>

#include <ftk/Core/FileIO.h>

#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Get the timeline file extensions.
        std::vector<std::string> getExtensions(
            const std::shared_ptr<ftk::Context>&,
            int types =
            static_cast<int>(io::FileType::Media) |
            static_cast<int>(io::FileType::Sequence));

        //! Convert frames to ranges.
        std::vector<OTIO_NS::TimeRange> toRanges(std::vector<OTIO_NS::RationalTime>);

        //! Loop a time.
        OTIO_NS::RationalTime loop(
            const OTIO_NS::RationalTime&,
            const OTIO_NS::TimeRange&,
            bool* looped = nullptr);

        //! Loop seconds.
        int64_t loop(
            int64_t,
            const OTIO_NS::TimeRange&,
            bool* looped = nullptr);

        //! Cache direction.
        enum class CacheDirection
        {
            Forward,
            Reverse,

            Count,
            First = Forward
        };
        FTK_ENUM(CacheDirection);

        //! Get the root (highest parent).
        const OTIO_NS::Composable* getRoot(const OTIO_NS::Composable*);

        //! Get the parent of the given type.
        template<typename T>
        const T* getParent(const OTIO_NS::Item*);

        //! Get the duration of all tracks of the same kind.
        std::optional<OTIO_NS::RationalTime> getDuration(
            const OTIO_NS::Timeline*,
            const std::string& kind);

        //! Get the time range of a timeline.
        OTIO_NS::TimeRange getTimeRange(const OTIO_NS::Timeline*);

        //! Get a list of paths to open from the given path.
        std::vector<file::Path> getPaths(
            const std::shared_ptr<ftk::Context>&,
            const file::Path&,
            const file::PathOptions&);

        //! Get an absolute path.
        file::Path getPath(
            const std::string& url,
            const std::string& directory,
            const file::PathOptions&);

        //! Get a path for a media reference.
        file::Path getPath(
            const OTIO_NS::MediaReference*,
            const std::string& directory,
            file::PathOptions);

        //! Get a memory read for a media reference.
        std::vector<ftk::InMemoryFile> getMemoryRead(
            const OTIO_NS::MediaReference*);

        //! Convert to memory references.
        enum class ToMemoryReference
        {
            Shared,
            Raw,

            Count,
            First = Shared
        };
        FTK_ENUM(ToMemoryReference);

        //! Convert media references to memory references for testing.
        void toMemoryReferences(
            OTIO_NS::Timeline*,
            const std::string& directory,
            ToMemoryReference,
            const file::PathOptions& = file::PathOptions());

        //! Transform track time to video media time.
        OTIO_NS::RationalTime toVideoMediaTime(
            const OTIO_NS::RationalTime&,
            const OTIO_NS::TimeRange& trimmedRangeInParent,
            const OTIO_NS::TimeRange& trimmedRange,
            double rate);

        //! Transform track time to audio media time.
        OTIO_NS::TimeRange toAudioMediaTime(
            const OTIO_NS::TimeRange&,
            const OTIO_NS::TimeRange& trimmedRangeInParent,
            const OTIO_NS::TimeRange& trimmedRange,
            double sampleRate);

        //! Copy audio data.
        std::vector<std::shared_ptr<audio::Audio> > audioCopy(
            const audio::Info&,
            const std::vector<AudioData>&,
            Playback,
            int64_t frame,
            int64_t size);

        //! Write a timeline to an .otioz file.
        bool writeOTIOZ(
            const std::string& fileName,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            const std::string& directory = std::string());
    }
}

#include <tlTimeline/UtilInline.h>
