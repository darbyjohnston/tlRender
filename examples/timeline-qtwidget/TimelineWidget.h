// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

#include "TimelineItem.h"

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

                math::Vector2i timelineSize() const;

            public Q_SLOTS:
                void setScale(float);

                void setThumbnailHeight(int);

                void setViewPos(const math::Vector2i&);

             Q_SIGNALS:
                void timelineSizeChanged(const math::Vector2i&);

            protected:
                void initializeGL() override;
                void resizeGL(int w, int h) override;
                void paintGL() override;

                void dragEnterEvent(QDragEnterEvent*) override;
                void dragMoveEvent(QDragMoveEvent*) override;
                void dragLeaveEvent(QDragLeaveEvent*) override;
                void dropEvent(QDropEvent*) override;

                void timerEvent(QTimerEvent*) override;

            private:
                void _tick(const std::shared_ptr<BaseItem>&);
                bool _doLayout(const std::shared_ptr<BaseItem>&);
                void _preLayout(const std::shared_ptr<BaseItem>&);
                bool _doRender(const std::shared_ptr<BaseItem>&);
                void _renderItems(
                    const std::shared_ptr<BaseItem>&,
                    const std::shared_ptr<timeline::IRender>&,
                    const math::BBox2i& viewport,
                    float devicePixelRatio);

                std::weak_ptr<system::Context> _context;
                std::shared_ptr<imaging::FontSystem> _fontSystem;
                math::Vector2i _viewPos;
                std::shared_ptr<TimelineItem> _timelineItem;
                math::Vector2i _timelineSize;
                std::shared_ptr<timeline::IRender> _render;
                int _timer = 0;
            };
        }
    }
}
