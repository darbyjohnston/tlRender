// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineScrollArea.h"

#include <QResizeEvent>
#include <QScrollBar>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            TimelineScrollArea::TimelineScrollArea(QWidget* parent) :
                QAbstractScrollArea(parent)
            {
            }

            void TimelineScrollArea::setTimelineWidget(TimelineWidget* widget)
            {
                if (widget == _timelineWidget)
                    return;
                delete _timelineWidget;
                _timelineWidget = widget;
                if (_timelineWidget)
                {
                    _timelineWidget->setParent(this);
                    _sizeUpdate();
                    connect(
                        _timelineWidget,
                        &TimelineWidget::timelineSizeChanged,
                        [this](const math::Vector2i& value)
                        {
                            _sizeUpdate();
                        });
                }
            }

            void TimelineScrollArea::resizeEvent(QResizeEvent* event)
            {
                if (_timelineWidget)
                {
                    _timelineWidget->resize(event->size());
                    _sizeUpdate();
                }
            }

            void TimelineScrollArea::_sizeUpdate()
            {
                if (_timelineWidget)
                {
                    const auto& timelineSize = _timelineWidget->timelineSize();
                    const math::Vector2i scrollSize(
                        _timelineWidget->width() - timelineSize.x,
                        _timelineWidget->height() - timelineSize.y);
                    horizontalScrollBar()->setMaximum(scrollSize.x);
                    verticalScrollBar()->setMaximum(scrollSize.y);
                }
            }
        }
    }
}
