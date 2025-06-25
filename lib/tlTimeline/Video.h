// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Transition.h>

#include <tlCore/Time.h>

#include <feather-tk/core/Image.h>
#include <feather-tk/core/RenderOptions.h>

namespace tl
{
    namespace timeline
    {
        //! Video layer.
        struct VideoLayer
        {
            std::shared_ptr<feather_tk::Image> image;
            feather_tk::ImageOptions imageOptions;

            std::shared_ptr<feather_tk::Image> imageB;
            feather_tk::ImageOptions imageOptionsB;

            Transition transition = Transition::None;
            float transitionValue = 0.F;

            bool operator == (const VideoLayer&) const;
            bool operator != (const VideoLayer&) const;
        };

        //! Video data.
        struct VideoData
        {
            feather_tk::Size2I size;
            OTIO_NS::RationalTime time = time::invalidTime;
            std::vector<VideoLayer> layers;

            bool operator == (const VideoData&) const;
            bool operator != (const VideoData&) const;
        };

        //! Compare the time values of video data.
        bool isTimeEqual(const VideoData&, const VideoData&);
    }
}
