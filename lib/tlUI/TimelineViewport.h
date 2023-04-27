// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlTimeline/TimelinePlayer.h>

namespace tl
{
    namespace ui
    {
        //! Timeline viewport.
        class TimelineViewport : public IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineViewport);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineViewport();

        public:
            ~TimelineViewport() override;

            //! Create a new timeline viewport.
            static std::shared_ptr<TimelineViewport> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the color configuration options.
            void setColorConfigOptions(const timeline::ColorConfigOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<timeline::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<std::shared_ptr<timeline::TimelinePlayer> >&);

            //! Get the view position.
            const math::Vector2i& viewPos() const;

            //! Get the view zoom.
            float viewZoom() const;

            //! Get whether the view is framed.
            bool hasFrameView() const;

            //! Set the view position and zoom.
            void setViewPosAndZoom(const math::Vector2i&, float);

            //! Set the view zoom.
            void setViewZoom(float, const math::Vector2i& focus = math::Vector2i());

            //! Frame the view.
            void frameView();

            //! Set the view zoom to 1:1.
            void viewZoom1To1();

            //! Zoom the view in.
            void viewZoomIn();

            //! Zoom the view out.
            void viewZoomOut();

            //! Set the view position and zoom callback.
            void setViewPosAndZoomCallback(
                const std::function<void(const math::Vector2i&, float)>&);

            //! Set the frame view callback.
            void setFrameViewCallback(const std::function<void(void)>&);

            void sizeEvent(const SizeEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            imaging::Size _renderSize() const;
            math::Vector2i _viewportCenter() const;
            void _frameView();

            TLRENDER_PRIVATE();
        };
    }
}
