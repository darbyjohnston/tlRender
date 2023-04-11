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

            const math::Vector2i& scrollSize() const;

            const math::Vector2i& scrollPos() const;

            void setTimelinePlayer(const std::shared_ptr<timeline::TimelinePlayer>&);

            void setStopOnScrub(bool);

            void setItemOptions(const ui::TimelineItemOptions&);

        public Q_SLOTS:
            void setScrollPos(const math::Vector2i&);
            void setScrollPosX(int);
            void setScrollPosY(int);

        Q_SIGNALS:
            void scrollSizeChanged(const math::Vector2i&);

            void scrollPosChanged(const tl::math::Vector2i&);

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

            void timerEvent(QTimerEvent*) override;

        private:
            void _setItemOptions(
                const std::shared_ptr<ui::IWidget>&,
                const ui::TimelineItemOptions&);

            math::BBox2i _timelineViewport() const;
            void _setViewport(
                const std::shared_ptr<ui::IWidget>&,
                const math::BBox2i&);

            TLRENDER_PRIVATE();
        };
    }
}
