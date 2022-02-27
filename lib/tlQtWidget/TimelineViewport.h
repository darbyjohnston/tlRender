// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/IRender.h>

#include <tlCore/OCIO.h>

#include <QOpenGLWidget>

namespace tl
{
    namespace qt
    {
        namespace widget
        {
            //! Timeline viewport widget.
            class TimelineViewport : public QOpenGLWidget
            {
                Q_OBJECT

            public:
                TimelineViewport(
                    const std::shared_ptr<core::system::Context>&,
                    QWidget* parent = nullptr);

                ~TimelineViewport() override;

                //! Set the color configuration.
                void setColorConfig(const core::imaging::ColorConfig&);

                //! Set the image options.
                void setImageOptions(const std::vector<timeline::ImageOptions>&);

                //! Set the comparison options.
                void setCompareOptions(const timeline::CompareOptions&);

                //! Set the timeline players.
                void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

                //! Get the view position.
                const core::math::Vector2i& viewPos() const;

                //! Get the view zoom.
                float viewZoom() const;

            public Q_SLOTS:
                //! Set the view position and zoom.
                void setViewPosAndZoom(const tl::core::math::Vector2i&, float);

                //! Set the view zoom.
                void setViewZoom(float, const tl::core::math::Vector2i& focus = tl::core::math::Vector2i());

                //! Frame the view.
                void frameView();

                //! Set the view zoom to 1:1.
                void viewZoom1To1();

                //! Zoom the view in.
                void viewZoomIn();

                //! Zoom the view out.
                void viewZoomOut();

            Q_SIGNALS:
                //! This signal is emitted when the position and zoom change.
                void viewPosAndZoomChanged(const tl::core::math::Vector2i&, float);

                //! This signal is emitted when the view is framed.
                void frameViewActivated();

            private Q_SLOTS:
                void _videoCallback(const tl::timeline::VideoData&);

            protected:
                void initializeGL() override;
                void resizeGL(int w, int h) override;
                void paintGL() override;
                void enterEvent(QEvent*) override;
                void leaveEvent(QEvent*) override;
                void mousePressEvent(QMouseEvent*) override;
                void mouseReleaseEvent(QMouseEvent*) override;
                void mouseMoveEvent(QMouseEvent*) override;
                void wheelEvent(QWheelEvent*) override;

            private:
                void _frameView();
                core::math::Vector2i _center() const;

                TLRENDER_PRIVATE();
            };
        }
    }
}
