// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace timelineui
    {
        //! Viewport background type.
        enum class ViewportBackground
        {
            Solid,
            Checkers,

            Count,
            First = Solid
        };
        TLRENDER_ENUM(ViewportBackground);
        TLRENDER_ENUM_SERIALIZE(ViewportBackground);

        //! Viewport background options.
        struct ViewportBackgroundOptions
        {
            ViewportBackground type           = ViewportBackground::Solid;
            image::Color4f     solidColor     = image::Color4f(0.F, 0.F, 0.F);
            image::Color4f     checkersColor0 = image::Color4f(1.F, 1.F, 1.F);
            image::Color4f     checkersColor1 = image::Color4f(0.F, 0.F, 0.F);
            math::Size2i       checkersSize   = math::Size2i(100, 100);

            bool operator == (const ViewportBackgroundOptions&) const;
            bool operator != (const ViewportBackgroundOptions&) const;
        };

        //! Timeline viewport.
        class TimelineViewport : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(TimelineViewport);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            TimelineViewport();

        public:
            virtual ~TimelineViewport();

            //! Create a new widget.
            static std::shared_ptr<TimelineViewport> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the background options.
            void setBackgroundOptions(const ViewportBackgroundOptions&);

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

            //! Set the comparison callback.
            void setCompareCallback(const std::function<void(timeline::CompareOptions)>&);

            //! Set the timeline players.
            void setPlayers(const std::vector<std::shared_ptr<timeline::Player> >&);

            //! Get the view position.
            const math::Vector2i& viewPos() const;

            //! Get the view zoom.
            double viewZoom() const;

            //! Set the view position and zoom.
            void setViewPosAndZoom(const math::Vector2i&, double);

            //! Set the view zoom.
            void setViewZoom(double, const math::Vector2i& focus = math::Vector2i());

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Observe whether the view is framed automatically.
            std::shared_ptr<observer::IValue<bool> > observeFrameView() const;

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set the view zoom to 1:1.
            void viewZoom1To1();

            //! Zoom the view in.
            void viewZoomIn();

            //! Zoom the view out.
            void viewZoomOut();

            //! Set the view position and zoom callback.
            void setViewPosAndZoomCallback(
                const std::function<void(const math::Vector2i&, double)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;
            void scrollEvent(ui::ScrollEvent&) override;
            void keyPressEvent(ui::KeyEvent&) override;
            void keyReleaseEvent(ui::KeyEvent&) override;

        protected:
            void _releaseMouse() override;

        private:
            math::Size2i _renderSize() const;
            math::Vector2i _viewportCenter() const;
            void _frameView();

            void _videoDataCallback(const timeline::VideoData&, size_t);

            TLRENDER_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const ViewportBackgroundOptions&);

        void from_json(const nlohmann::json&, ViewportBackgroundOptions&);

        ///@}
    }
}
