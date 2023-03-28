// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "TimelineItem.h"

#include <tlUI/EventLoop.h>
#include <tlUI/ScrollArea.h>

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

                const math::Vector2i& scrollSize() const;

                const math::Vector2i& scrollPos() const;

                void setTimeline(const std::shared_ptr<timeline::Timeline>&);

            public Q_SLOTS:
                void setScale(float);

                void setThumbnailHeight(int);

                void setScrollPos(const math::Vector2i&);
                void setScrollPosX(int);
                void setScrollPosY(int);

             Q_SIGNALS:
                void scrollSizeChanged(const math::Vector2i&);

                void scrollPosChanged(const tl::math::Vector2i&);

                void currentTimeChanged(const otime::RationalTime&);

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

                std::shared_ptr<imaging::FontSystem> _fontSystem;
                std::shared_ptr<ui::IconLibrary> _iconLibrary;
                std::shared_ptr<ui::Style> _style;
                std::shared_ptr<ui::EventLoop> _eventLoop;
                std::shared_ptr<ui::ScrollArea> _scrollArea;
                math::Vector2i _scrollSize;
                math::Vector2i _scrollPos;
                std::shared_ptr<observer::ValueObserver<math::Vector2i> > _scrollSizeObserver;
                std::shared_ptr<observer::ValueObserver<math::Vector2i> > _scrollPosObserver;
                std::shared_ptr<TimelineItem> _timelineItem;
                std::shared_ptr<observer::ValueObserver<otime::RationalTime> > _currentTimeObserver;
                std::shared_ptr<timeline::IRender> _render;

                int _timer = 0;
            };
        }
    }
}
