// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelineOptions.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/String.h>

#include <sstream>

namespace tl
{
    namespace timeline
    {
        FEATHER_TK_ENUM_IMPL(
            FileSequenceAudio,
            "None",
            "Extension",
            "FileName");

        bool Options::operator == (const Options& other) const
        {
            return
                fileSequenceAudio == other.fileSequenceAudio &&
                fileSequenceAudioExtensions == other.fileSequenceAudioExtensions &&
                fileSequenceAudioFileName == other.fileSequenceAudioFileName &&
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
