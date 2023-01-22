// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/IRender.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace qtwidget
    {
        //! Timeline viewport widget.
        class TimelineViewport :
            public QOpenGLWidget,
            protected QOpenGLFunctions_4_1_Core
        {
            Q_OBJECT

        public:
            TimelineViewport(
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            ~TimelineViewport() override;

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
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

            //! Get the view position.
            const math::Vector2i& viewPos() const;

            //! Get the view zoom.
            float viewZoom() const;

            //! Get whether the view is framed.
            bool hasFrameView() const;

        public Q_SLOTS:
            //! Set the view position and zoom.
            void setViewPosAndZoom(const tl::math::Vector2i&, float);

            //! Set the view zoom.
            void setViewZoom(float, const tl::math::Vector2i& focus = tl::math::Vector2i());

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
            void viewPosAndZoomChanged(const tl::math::Vector2i&, float);

            //! This signal is emitted when the view is framed.
            void frameViewActivated();

        private Q_SLOTS:
            void _currentVideoCallback(const tl::timeline::VideoData&);

        protected:
            void initializeGL() override;
            void resizeGL(int w, int h) override;
            void paintGL() override;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            void enterEvent(QEvent*) override;
#else
            void enterEvent(QEnterEvent*) override;
#endif // QT_VERSION
            void leaveEvent(QEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;
            void wheelEvent(QWheelEvent*) override;

        private:
            imaging::Size _getViewportSize() const;
            imaging::Size _getRenderSize() const;
            math::Vector2i _getViewportCenter() const;
            void _frameView();

            TLRENDER_PRIVATE();
        };
    }
}
