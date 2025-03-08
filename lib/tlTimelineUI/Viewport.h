// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/ColorOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ForegroundOptions.h>
#include <tlTimeline/Player.h>

#include <dtk/ui/IWidget.h>

namespace tl
{
    namespace timelineui
    {
        //! Timeline viewport.
        class Viewport : public dtk::IWidget
        {
            DTK_NON_COPYABLE(Viewport);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Viewport();

        public:
            virtual ~Viewport();

            //! Create a new widget.
            static std::shared_ptr<Viewport> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the comparison callback.
            void setCompareCallback(const std::function<void(timeline::CompareOptions)>&);

            //! Set the OpenColorIO options.
            void setOCIOOptions(const timeline::OCIOOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<dtk::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Set the foreground options.
            void setForegroundOptions(const timeline::ForegroundOptions&);

            //! Get the color buffer type.
            dtk::ImageType getColorBuffer() const;

            //! Observe the color buffer type.
            std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > observeColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(dtk::ImageType);

            //! Get the timeline player.
            const std::shared_ptr<timeline::Player>& getPlayer() const;

            //! Set the timeline player.
            virtual void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Get the view position.
            const dtk::V2I& getViewPos() const;

            //! Get the view zoom.
            double getViewZoom() const;

            //! Set the view position and zoom.
            void setViewPosAndZoom(const dtk::V2I&, double);

            //! Set the view zoom.
            void setViewZoom(double, const dtk::V2I& focus = dtk::V2I());

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Observe whether the view is framed automatically.
            std::shared_ptr<dtk::IObservableValue<bool> > observeFrameView() const;

            //! Set whether the view is framed automatically.
            void setFrameView(bool);

            //! Set the view framed callback.
            void setFrameViewCallback(const std::function<void(bool)>&);

            //! Reset the view zoom to 1:1.
            void viewZoomReset();

            //! Zoom the view in.
            void viewZoomIn();

            //! Zoom the view out.
            void viewZoomOut();

            //! Set the view position and zoom callback.
            void setViewPosAndZoomCallback(
                const std::function<void(const dtk::V2I&, double)>&);

            //! Get the frames per second.
            double getFPS() const;

            //! Observe the frames per second.
            std::shared_ptr<dtk::IObservableValue<double> > observeFPS() const;

            //! Get the number of dropped frames during playback.
            size_t getDroppedFrames() const;

            //! Observe the number of dropped frames during playback.
            std::shared_ptr<dtk::IObservableValue<size_t> > observeDroppedFrames() const;
            
            //! Sample a color from the viewport.
            dtk::Color4F getColorSample(const dtk::V2I&);

            //! Set the keyboard modifier for panning.
            void setPanModifier(dtk::KeyModifier);

            //! Set the keyboard modifier for wiping.
            void setWipeModifier(dtk::KeyModifier);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void drawEvent(const dtk::Box2I&, const dtk::DrawEvent&) override;
            void mouseMoveEvent(dtk::MouseMoveEvent&) override;
            void mousePressEvent(dtk::MouseClickEvent&) override;
            void mouseReleaseEvent(dtk::MouseClickEvent&) override;
            void scrollEvent(dtk::ScrollEvent&) override;
            void keyPressEvent(dtk::KeyEvent&) override;
            void keyReleaseEvent(dtk::KeyEvent&) override;

        protected:
            void _releaseMouse() override;

        private:
            dtk::Size2I _getRenderSize() const;
            dtk::V2I _getViewportCenter() const;
            void _frameView();

            void _droppedFramesUpdate(const OTIO_NS::RationalTime&);

            DTK_PRIVATE();
        };
    }
}
