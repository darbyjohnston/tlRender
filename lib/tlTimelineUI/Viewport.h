// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/ColorOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ForegroundOptions.h>
#include <tlTimeline/Player.h>

#include <feather-tk/ui/IWidget.h>

namespace tl
{
    namespace timelineui
    {
        //! Timeline viewport.
        class Viewport : public feather_tk::IWidget
        {
            FEATHER_TK_NON_COPYABLE(Viewport);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Viewport();

        public:
            virtual ~Viewport();

            //! Create a new widget.
            static std::shared_ptr<Viewport> create(
                const std::shared_ptr<feather_tk::Context>&,
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
            void setImageOptions(const std::vector<feather_tk::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Set the foreground options.
            void setForegroundOptions(const timeline::ForegroundOptions&);

            //! Get the color buffer type.
            feather_tk::ImageType getColorBuffer() const;

            //! Observe the color buffer type.
            std::shared_ptr<feather_tk::IObservableValue<feather_tk::ImageType> > observeColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(feather_tk::ImageType);

            //! Get the timeline player.
            const std::shared_ptr<timeline::Player>& getPlayer() const;

            //! Set the timeline player.
            virtual void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Get the view position.
            const feather_tk::V2I& getViewPos() const;

            //! Get the view zoom.
            double getViewZoom() const;

            //! Set the view position and zoom.
            void setViewPosAndZoom(const feather_tk::V2I&, double);

            //! Set the view zoom.
            void setViewZoom(double, const feather_tk::V2I& focus = feather_tk::V2I());

            //! Get whether the view is framed automatically.
            bool hasFrameView() const;

            //! Observe whether the view is framed automatically.
            std::shared_ptr<feather_tk::IObservableValue<bool> > observeFrameView() const;

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
                const std::function<void(const feather_tk::V2I&, double)>&);

            //! Get the frames per second.
            double getFPS() const;

            //! Observe the frames per second.
            std::shared_ptr<feather_tk::IObservableValue<double> > observeFPS() const;

            //! Get the number of dropped frames during playback.
            size_t getDroppedFrames() const;

            //! Observe the number of dropped frames during playback.
            std::shared_ptr<feather_tk::IObservableValue<size_t> > observeDroppedFrames() const;
            
            //! Sample a color from the viewport.
            feather_tk::Color4F getColorSample(const feather_tk::V2I&);

            //! Set the keyboard modifier for panning.
            void setPanModifier(feather_tk::KeyModifier);

            //! Set the keyboard modifier for wiping.
            void setWipeModifier(feather_tk::KeyModifier);

            void setGeometry(const feather_tk::Box2I&) override;
            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
            void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;
            void mouseMoveEvent(feather_tk::MouseMoveEvent&) override;
            void mousePressEvent(feather_tk::MouseClickEvent&) override;
            void mouseReleaseEvent(feather_tk::MouseClickEvent&) override;
            void scrollEvent(feather_tk::ScrollEvent&) override;
            void keyPressEvent(feather_tk::KeyEvent&) override;
            void keyReleaseEvent(feather_tk::KeyEvent&) override;

        protected:
            void _releaseMouse() override;

        private:
            feather_tk::Size2I _getRenderSize() const;
            feather_tk::V2I _getViewportCenter() const;
            void _frameView();

            void _droppedFramesUpdate(const OTIO_NS::RationalTime&);

            FEATHER_TK_PRIVATE();
        };
    }
}
