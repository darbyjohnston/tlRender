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
        struct TimelineScrollArea::Private
        {
            TimelineWidget* timelineWidget = nullptr;
        };

        TimelineScrollArea::TimelineScrollArea(QWidget* parent) :
            QAbstractScrollArea(parent),
            _p(new Private)
        {
            connect(
                horizontalScrollBar(),
                &QAbstractSlider::valueChanged,
                [this](int value)
                {
                    if (_p->timelineWidget)
                    {
                        _p->timelineWidget->setScrollPosX(value);
                    }
                });
            connect(
                verticalScrollBar(),
                &QAbstractSlider::valueChanged,
                [this](int value)
                {
                    if (_p->timelineWidget)
                    {
                        _p->timelineWidget->setScrollPosY(value);
                    }
                });
        }

        TimelineScrollArea::~TimelineScrollArea()
        {}

        void TimelineScrollArea::setTimelineWidget(TimelineWidget* widget)
        {
            TLRENDER_P();
            if (widget == p.timelineWidget)
                return;
            delete p.timelineWidget;
            p.timelineWidget = widget;
            if (p.timelineWidget)
            {
                p.timelineWidget->setParent(this);
                _sizeUpdate();
                connect(
                    p.timelineWidget,
                    &TimelineWidget::scrollSizeChanged,
                    [this](const math::Vector2i& value)
                    {
                        _sizeUpdate();
                    });
                connect(
                    p.timelineWidget,
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
            TLRENDER_P();
            if (p.timelineWidget)
            {
                p.timelineWidget->resize(event->size());
                _sizeUpdate();
            }
        }

        void TimelineScrollArea::_sizeUpdate()
        {
            TLRENDER_P();
            if (p.timelineWidget)
            {
                const auto& scrollSize = p.timelineWidget->scrollSize();
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
