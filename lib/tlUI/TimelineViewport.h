// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

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

            //! Create a new widget.
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
            void setPlayers(const std::vector<std::shared_ptr<timeline::Player> >&);

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
            void setFrameViewCallback(const std::function<void(bool)>&);

            void setVisible(bool) override;
            void setEnabled(bool) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            imaging::Size _renderSize() const;
            math::Vector2i _viewportCenter() const;
            void _frameView();

            void _resetMouse();

            TLRENDER_PRIVATE();
        };
    }
}
