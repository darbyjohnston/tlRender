// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

namespace tl
{
    namespace timeline
    {
        //! Audio layer.
        struct AudioLayer
        {
            std::shared_ptr<audio::Audio> audio;

            bool operator == (const AudioLayer&) const;
            bool operator != (const AudioLayer&) const;
        };

        //! Audio data.
        struct AudioData
        {
            double seconds = -1.0;
            std::vector<AudioLayer> layers;

            bool operator == (const AudioData&) const;
            bool operator != (const AudioData&) const;
        };

        //! Compare the time values of audio data.
        bool isTimeEqual(const AudioData&, const AudioData&);
    }
}
