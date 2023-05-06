// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/TimelineItem.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace qtwidget
    {
        //! Timeline widget.
        class TimelineWidget :
            public QOpenGLWidget,
            protected QOpenGLFunctions_4_1_Core
        {
            Q_OBJECT

        public:
            TimelineWidget(
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            ~TimelineWidget() override;

            //! Set the timeline player.
            void setTimelinePlayer(const std::shared_ptr<timeline::TimelinePlayer>&);

            //! Get the item options.
            const ui::TimelineItemOptions& itemOptions() const;

        public Q_SLOTS:
            //! Set the view zoom.
            void setViewZoom(float);

            //! Set the view zoom.
            void setViewZoom(
                float,
                const tl::math::Vector2i& focus);

            //! Set whether the to frame the view.
            void setFrameView(bool);

            //! Set whether to stop playback when scrubbing.
            void setStopOnScrub(bool);

            //! Set the mouse wheel scale.
            void setMouseWheelScale(float);

            //! Set the item options.
            void setItemOptions(const ui::TimelineItemOptions&);

        Q_SIGNALS:
            //! This signal is emitted when the frame view is changed.
            void frameViewChanged(bool);

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
            void keyPressEvent(QKeyEvent*) override;
            void keyReleaseEvent(QKeyEvent*) override;

            void timerEvent(QTimerEvent*) override;

        private:
            void _frameView();

            void _setViewZoom(
                float zoomNew,
                float zoomPrev,
                const math::Vector2i& focus,
                const math::Vector2i& scrollPos);

            float _timelineScale() const;
            void _setItemOptions(
                const std::shared_ptr<ui::IWidget>&,
                const ui::TimelineItemOptions&);

            int _toUI(int) const;
            math::Vector2i _toUI(const math::Vector2i&) const;
            int _fromUI(int) const;
            math::Vector2i _fromUI(const math::Vector2i&) const;

            TLRENDER_PRIVATE();
        };
    }
}
