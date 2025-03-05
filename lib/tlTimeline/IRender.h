// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/ColorOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ForegroundOptions.h>
#include <tlTimeline/Video.h>

#include <dtk/core/IRender.h>

namespace tl
{
    namespace timeline
    {
        //! Base class for renderers.
        class IRender : public dtk::IRender
        {
        public:
            virtual ~IRender() = 0;

            //! Set the OpenColorIO options.
            virtual void setOCIOOptions(const OCIOOptions&) = 0;

            //! Set the LUT options.
            virtual void setLUTOptions(const LUTOptions&) = 0;

            //! Draw the background.
            virtual void drawBackground(
                const std::vector<dtk::Box2I>&,
                const dtk::M44F&,
                const BackgroundOptions&) = 0;

            //! Draw timeline video data.
            virtual void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>& = {},
                const std::vector<DisplayOptions>& = {},
                const CompareOptions& = CompareOptions()) = 0;

            //! Draw the foreground.
            virtual void drawForeground(
                const std::vector<dtk::Box2I>&,
                const dtk::M44F&,
                const ForegroundOptions&) = 0;
        };
    }
}
