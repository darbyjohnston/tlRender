// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>
#include <tlCore/Time.h>

#include <feather-tk/core/Image.h>

namespace tl
{
    //! Audio and video I/O.
    namespace io
    {
        //! File types.
        enum class FileType
        {
            Unknown  = 0,
            Media    = 1,
            Sequence = 2,

            Count,
            First = Unknown
        };

        //! I/O information.
        struct Info
        {
            //! Video layer information.
            std::vector<feather_tk::ImageInfo> video;

            //! Video time range.
            OTIO_NS::TimeRange videoTime = time::invalidTimeRange;

            //! Audio information.
            audio::Info audio;

            //! Audio time range.
            OTIO_NS::TimeRange audioTime = time::invalidTimeRange;

            //! Metadata tags.
            feather_tk::ImageTags tags;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Video I/O data.
        struct VideoData
        {
            VideoData();
            VideoData(
                const OTIO_NS::RationalTime&,
                uint16_t layer,
                const std::shared_ptr<feather_tk::Image>&);

            OTIO_NS::RationalTime       time = time::invalidTime;
            uint16_t                    layer = 0;
            std::shared_ptr<feather_tk::Image> image;

            bool operator == (const VideoData&) const;
            bool operator != (const VideoData&) const;
            bool operator < (const VideoData&) const;
        };

        //! Audio I/O data.
        struct AudioData
        {
            AudioData();
            AudioData(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<audio::Audio>&);

            OTIO_NS::RationalTime         time = time::invalidTime;
            std::shared_ptr<audio::Audio> audio;

            bool operator == (const AudioData&) const;
            bool operator != (const AudioData&) const;
            bool operator < (const AudioData&) const;
        };

        //! Get an integer image type for the given channel count and bit depth.
        feather_tk::ImageType getIntType(size_t channelCount, size_t bitDepth);

        //! Get a floating point image type for the given channel count and bit
        //! depth.
        feather_tk::ImageType getFloatType(size_t channelCount, size_t bitDepth);

        //! Options.
        typedef std::map<std::string, std::string> Options;

        //! Merge options.
        Options merge(const Options&, const Options&);
    }
}

#include <tlIO/IOInline.h>
