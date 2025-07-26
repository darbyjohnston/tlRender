// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Path.h>

namespace tl
{
    namespace io
    {
        //! Get an I/O information cache key.
        std::string getInfoCacheKey(
            const file::Path&,
            const Options&);

        //! Get a video cache key.
        std::string getVideoCacheKey(
            const file::Path&,
            const OTIO_NS::RationalTime&,
            const Options& initOptions,
            const Options& frameOptions);

        //! Get an audio cache key.
        std::string getAudioCacheKey(
            const file::Path&,
            const OTIO_NS::TimeRange&,
            const Options& initOptions,
            const Options& frameOptions);
    }
}
