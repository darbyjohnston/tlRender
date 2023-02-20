// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineItem.h"

#include "StackItem.h"

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            TimelineItem::TimelineItem(
                const otio::Timeline* timeline,
                const ItemOptions& options,
                QGraphicsItem* parent) :
                BaseItem(options, parent)
            {
                _label = QString("Timeline: %1").arg(QString::fromUtf8(timeline->name().c_str()));

                _duration = timeline->duration();

                _stackItem = new StackItem(timeline->tracks(), options);
                _stackItem->setParentItem(this);
            }

            void TimelineItem::layout()
            {
                _stackItem->layout();
                _stackItem->setY(_options.margin + _options.fontLineSize + _options.margin);
            }

            QRectF TimelineItem::boundingRect() const
            {
                return QRectF(
                    0,
                    0,
                    _duration.rescaled_to(1.0).value() * _zoom.x,
                    _options.margin + _options.fontLineSize + _options.margin +
                        (_stackItem ? _stackItem->boundingRect().height() : 0));
            }

            void TimelineItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(63, 63, 63));
                painter->drawRect(
                    0,
                    0,
                    _duration.rescaled_to(1.0).value() * _zoom.x,
                    _options.margin + _options.fontLineSize + _options.margin +
                        (_stackItem ? _stackItem->boundingRect().height() : 0));

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _options.margin,
                    _options.margin + _options.fontLineSize - _options.fontDescender,
                    _label);
            }
        }
    }
}
