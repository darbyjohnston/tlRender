// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Transition.h>

#include <tlCore/Time.h>

#include <ftk/Core/Image.h>
#include <ftk/Core/RenderOptions.h>

namespace tl
{
    namespace timeline
    {
        //! Video layer.
        struct VideoLayer
        {
            std::shared_ptr<ftk::Image> image;
            ftk::ImageOptions imageOptions;

            std::shared_ptr<ftk::Image> imageB;
            ftk::ImageOptions imageOptionsB;

            Transition transition = Transition::None;
            float transitionValue = 0.F;

            bool operator == (const VideoLayer&) const;
            bool operator != (const VideoLayer&) const;
        };

        //! Video data.
        struct VideoData
        {
            ftk::Size2I size;
            OTIO_NS::RationalTime time = time::invalidTime;
            std::vector<VideoLayer> layers;

            bool operator == (const VideoData&) const;
            bool operator != (const VideoData&) const;
        };

        //! Compare the time values of video data.
        bool isTimeEqual(const VideoData&, const VideoData&);
    }
}
