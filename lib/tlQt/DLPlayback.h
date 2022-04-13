// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/IRender.h>

#include <tlCore/Image.h>
#include <tlCore/OCIO.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace qt
    {
        //! DeckLink playback.
        class DLPlayback : public QObject
        {
            Q_OBJECT

        public:
            DLPlayback(
                int deviceIndex,
                const std::shared_ptr<system::Context>&);

            ~DLPlayback();

            //! Set the color configuration.
            void setColorConfig(const imaging::ColorConfig&);

            //! Set the image options.
            void setImageOptions(const std::vector<timeline::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

            //! Get the view position.
            const math::Vector2i& viewPos() const;

            //! Get the view zoom.
            float viewZoom() const;

        public Q_SLOTS:
            //! Set the view position and zoom.
            void setViewPosAndZoom(const tl::math::Vector2i&, float);

            //! Set the view zoom.
            void setViewZoom(float, const tl::math::Vector2i& focus = tl::math::Vector2i());

            //! Frame the view.
            void frameView();

        private Q_SLOTS:
            void _videoCallback(const tl::timeline::VideoData&);

        private:
            void _frameView();
            void _render();

            TLRENDER_PRIVATE();
        };
    }
}
