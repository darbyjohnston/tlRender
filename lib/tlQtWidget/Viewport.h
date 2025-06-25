// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/ContainerWidget.h>

#include <tlQt/PlayerObject.h>

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/ColorOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ForegroundOptions.h>

#include <QSharedPointer>
#include <QVector>

namespace tl
{
    namespace qtwidget
    {
        //! Timeline viewport widget.
        class Viewport : public ContainerWidget
        {
            Q_OBJECT

        public:
            Viewport(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::Style>&,
                QWidget* parent = nullptr);

            virtual ~Viewport();

            //! Get the color buffer type.
            feather_tk::ImageType colorBuffer() const;

            //! Get the view position.
            const feather_tk::V2I& viewPos() const;

            //! Get the view zoom.
            double viewZoom() const;

            //! Get whether the view is framed.
            bool hasFrameView() const;

            //! Get the frames per second.
            double getFPS() const;

            //! Get the number of dropped frames during playback.
            size_t getDroppedFrames() const;

        public Q_SLOTS:
            //! Set the OpenColorIO options.
            void setOCIOOptions(const timeline::OCIOOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<feather_tk::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Set the foreground options.
            void setForegroundOptions(const timeline::ForegroundOptions&);

            //! Set the color buffer type.
            void setColorBuffer(feather_tk::ImageType);

            //! Set the timeline player.
            void setPlayer(const QSharedPointer<qt::PlayerObject>&);

            //! Set the view position and zoom.
            void setViewPosAndZoom(const feather_tk::V2I&, double);

            //! Set the view zoom.
            void setViewZoom(double, const feather_tk::V2I& focus = feather_tk::V2I());

            //! Frame the view.
            void setFrameView(bool);

            //! Reset the view zoom to 1:1.
            void viewZoomReset();

            //! Zoom the view in.
            void viewZoomIn();

            //! Zoom the view out.
            void viewZoomOut();

        Q_SIGNALS:
            //! This signal is emitted when the comparison options are changed.
            void compareOptionsChanged(const tl::timeline::CompareOptions&);

            //! This signal is emitted when the position and zoom change.
            void viewPosAndZoomChanged(const feather_tk::V2I&, double);

            //! This signal is emitted when the frame view is changed.
            void frameViewChanged(bool);

            //! This signal is emitetd when the FPS is changed.
            void fpsChanged(double);

            //! This signal is emitted when the dropped frames count is changed.
            void droppedFramesChanged(size_t);

            //! This signal is emitted when the color picker is changed.
            void colorPickerChanged(const feather_tk::Color4F&);

        private:
            FEATHER_TK_PRIVATE();
        };
    }
}
