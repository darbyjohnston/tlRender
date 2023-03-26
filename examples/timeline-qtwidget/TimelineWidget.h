// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "TimelineItem.h"

#include <tlUI/EventLoop.h>

#include <tlTimeline/Timeline.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
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

                void setTimeline(const std::shared_ptr<timeline::Timeline>&);

                const math::Vector2i& timelineSize() const;

            public Q_SLOTS:
                void setScale(float);

                void setThumbnailHeight(int);

                void setViewPos(const math::Vector2i&);
                void setViewPosX(int);
                void setViewPosY(int);

             Q_SIGNALS:
                void timelineSizeChanged(const math::Vector2i&);

                void viewPosChanged(const tl::math::Vector2i&);

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
                math::BBox2i _timelineViewport() const;

                std::weak_ptr<system::Context> _context;
                math::Vector2i _viewPos;
                std::shared_ptr<TimelineItem> _timelineItem;
                math::Vector2i _timelineSize;
                std::shared_ptr<observer::ValueObserver<math::Vector2i> > _timelineSizeObserver;

                std::shared_ptr<imaging::FontSystem> _fontSystem;
                std::shared_ptr<ui::IconLibrary> _iconLibrary;
                std::shared_ptr<ui::Style> _style;
                std::shared_ptr<ui::EventLoop> _eventLoop;
                std::shared_ptr<timeline::IRender> _render;

                int _timer = 0;
            };
        }
    }
}
