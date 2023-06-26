// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <opentimelineio/mediaReference.h>

namespace tl
{
    namespace timeline
    {
        //! Convert frames to ranges.
        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime>);

        //! Loop a time.
        otime::RationalTime loop(
            const otime::RationalTime&,
            const otime::TimeRange&,
            bool* looped = nullptr);

        //! Loop a range.
        std::vector<otime::TimeRange> loop(
            const otime::TimeRange&,
            const otime::TimeRange&);

        //! Get the root (highest parent).
        const otio::Composable* getRoot(const otio::Composable*);

        //! Get the parent of the given type.
        template<typename T>
        const T* getParent(const otio::Item*);

        //! Get the duration of all tracks of the same kind.
        otio::optional<otime::RationalTime> getDuration(
            const otio::Timeline*,
            const std::string& kind);

        //! Get the time range of a timeline.
        otime::TimeRange getTimeRange(const otio::Timeline*);

        //! Get a list of files to open from the given path.
        std::vector<file::Path> getPaths(
            const std::string&,
            const file::PathOptions&,
            const std::shared_ptr<system::Context>&);

        //! Remove a "file://" prefix.
        std::string removeFileURLPrefix(const std::string&);

        //! Get an absolute path.
        file::Path getPath(
            const std::string& url,
            const std::string& directory,
            const file::PathOptions&);

        //! Get a path for a media reference.
        file::Path getPath(
            const otio::MediaReference*,
            const std::string& directory,
            file::PathOptions);

        //! Get a memory read for a media reference.
        std::vector<file::MemoryRead> getMemoryRead(
            const otio::MediaReference*);

        //! Transform track time to video media time.
        otime::RationalTime toVideoMediaTime(
            const otime::RationalTime&,
            const otio::Track*,
            const otio::Clip*,
            const io::Info&);

        //! Transform track time to audio media time.
        otime::TimeRange toAudioMediaTime(
            const otime::TimeRange&,
            const otio::Track*,
            const otio::Clip*,
            const io::Info&);
    }
}

#include <tlTimeline/UtilInline.h>
