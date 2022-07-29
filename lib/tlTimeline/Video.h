// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/RenderOptions.h>

#include <tlCore/Context.h>
#include <tlCore/Time.h>

#include <opentimelineio/anyDictionary.h>

namespace tl
{
    namespace imaging
    {
        class FontSystem;
    }

    namespace timeline
    {
        class IRender;

        //! Transitions.
        enum class Transition
        {
            None,
            Dissolve,

            Count,
            First = None
        };
        TLRENDER_ENUM(Transition);
        TLRENDER_ENUM_SERIALIZE(Transition);

        //! Convert to a transition.
        Transition toTransition(const std::string&);

        //! Video layer.
        struct VideoLayer
        {
            std::shared_ptr<imaging::Image> image;
            ImageOptions imageOptions;

            std::shared_ptr<imaging::Image> imageB;
            ImageOptions imageOptionsB;

            Transition transition = Transition::None;
            float transitionValue = 0.F;

            bool operator == (const VideoLayer&) const;
            bool operator != (const VideoLayer&) const;
        };

        //! Video data.
        struct VideoData
        {
            otime::RationalTime time = time::invalidTime;
            std::vector<VideoLayer> layers;
            DisplayOptions displayOptions;

            bool operator == (const VideoData&) const;
            bool operator != (const VideoData&) const;
        };

        //! Compare the time values of video data.
        bool isTimeEqual(const VideoData&, const VideoData&);
    }
}

#include <tlTimeline/VideoInline.h>
