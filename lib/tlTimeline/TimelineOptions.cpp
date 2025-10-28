// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelineOptions.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

#include <sstream>

namespace tl
{
    namespace timeline
    {
        FTK_ENUM_IMPL(
            ImageSequenceAudio,
            "None",
            "Extension",
            "FileName");

        bool Options::operator == (const Options& other) const
        {
            return
                imageSequenceAudio == other.imageSequenceAudio &&
                imageSequenceAudioExtensions == other.imageSequenceAudioExtensions &&
                imageSequenceAudioFileName == other.imageSequenceAudioFileName &&
                compat == other.compat &&
                videoRequestMax == other.videoRequestMax &&
                audioRequestMax == other.audioRequestMax &&
                requestTimeout == other.requestTimeout &&
                ioOptions == other.ioOptions &&
                pathOptions == other.pathOptions;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }
    }
}
