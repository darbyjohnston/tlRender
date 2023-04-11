// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineScrollArea.h>

#include <QResizeEvent>
#include <QScrollBar>

namespace tl
{
    namespace qtwidget
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
                        _timelineWidget->setScrollPosX(value);
                    }
                });
            connect(
                verticalScrollBar(),
                &QAbstractSlider::valueChanged,
                [this](int value)
                {
                    if (_timelineWidget)
                    {
                        _timelineWidget->setScrollPosY(value);
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
                    &TimelineWidget::scrollSizeChanged,
                    [this](const math::Vector2i& value)
                    {
                        _sizeUpdate();
                    });
                connect(
                    _timelineWidget,
                    &TimelineWidget::scrollPosChanged,
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
                const auto& scrollSize = _timelineWidget->scrollSize();
                const math::Vector2i viewportSize(
                    viewport()->width(),
                    viewport()->height());
                const math::Vector2i scrollRange(
                    std::max(0, scrollSize.x - viewportSize.x),
                    std::max(0, scrollSize.y - viewportSize.y));
                horizontalScrollBar()->setRange(0, scrollRange.x);
                horizontalScrollBar()->setPageStep(viewportSize.x);
                horizontalScrollBar()->setSingleStep(10);
                verticalScrollBar()->setRange(0, scrollRange.y);
                verticalScrollBar()->setPageStep(viewportSize.y);
                verticalScrollBar()->setSingleStep(10);
            }
        }
    }
}
