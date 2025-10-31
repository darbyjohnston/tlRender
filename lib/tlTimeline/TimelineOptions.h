// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Path.h>

namespace tl
{
    //! Timelines.
    namespace timeline
    {
        //! Image sequence audio options.
        enum class ImageSequenceAudio
        {
            None,      //!< No audio
            Extension, //!< Search for an audio file by extension
            FileName,  //!< Use the given audio file name

            Count,
            First = None
        };
        FTK_ENUM(ImageSequenceAudio);

        //! Timeline options.
        struct Options
        {
            //! Image sequence audio.
            ImageSequenceAudio imageSequenceAudio = ImageSequenceAudio::Extension;

            //! Image sequence audio extensions.
            std::vector<std::string> imageSequenceAudioExtensions = { ".mp3", ".wav" };

            //! Image sequence audio file name.
            std::string imageSequenceAudioFileName;

            //! Enable workarounds for timelines that may not conform exactly
            //! to specification.
            bool compat = true;

            //! Maximum number of video requests.
            size_t videoRequestMax = 16;

            //! Maximum number of audio requests.
            size_t audioRequestMax = 16;

            //! Request timeout.
            std::chrono::milliseconds requestTimeout = std::chrono::milliseconds(5);

            //! I/O options.
            io::Options ioOptions;

            //! Path options.
            file::PathOptions pathOptions;

            bool operator == (const Options&) const;
            bool operator != (const Options&) const;
        };
    }
}
