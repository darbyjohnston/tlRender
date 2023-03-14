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
                connect(
                    horizontalScrollBar(),
                    &QAbstractSlider::valueChanged,
                    [this](int value)
                    {
                        if (_timelineWidget)
                        {
                            _timelineWidget->setViewPosX(value);
                        }
                    });
                connect(
                    verticalScrollBar(),
                    &QAbstractSlider::valueChanged,
                    [this](int value)
                    {
                        if (_timelineWidget)
                        {
                            _timelineWidget->setViewPosY(value);
                        }
                    });
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
                    connect(
                        _timelineWidget,
                        &TimelineWidget::viewPosChanged,
                        [this](const math::Vector2i& value)
                        {
                            horizontalScrollBar()->setValue(value.x);
                            verticalScrollBar()->setValue(value.y);
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
                    const math::Vector2i viewportSize(
                        viewport()->width(),
                        viewport()->height());
                    const math::Vector2i scrollSize(
                        std::max(0, timelineSize.x - viewportSize.x),
                        std::max(0, timelineSize.y - viewportSize.y));
                    horizontalScrollBar()->setRange(0, scrollSize.x);
                    horizontalScrollBar()->setPageStep(viewportSize.x);
                    horizontalScrollBar()->setSingleStep(10);
                    verticalScrollBar()->setRange(0, scrollSize.y);
                    verticalScrollBar()->setPageStep(viewportSize.y);
                    verticalScrollBar()->setSingleStep(10);
                }
            }
        }
    }
}
