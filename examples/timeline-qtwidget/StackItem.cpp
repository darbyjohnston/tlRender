// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "StackItem.h"

#include "TrackItem.h"

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            StackItem::StackItem(
                const otio::Stack* stack,
                const ItemOptions& options,
                QGraphicsItem* parent) :
                BaseItem(options, parent)
            {
                _label = QString("Stack: %1").arg(QString::fromUtf8(stack->name().c_str()));

                _timeRange = stack->trimmed_range();

                for (const auto& child : stack->children())
                {
                    if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                    {
                        auto trackItem = new TrackItem(track, options);
                        trackItem->setParentItem(this);
                        _trackItems.push_back(trackItem);
                    }
                }
            }

            void StackItem::layout()
            {
                qreal y = _options.margin + _options.fontLineSize + _options.margin;
                for (auto item : _trackItems)
                {
                    item->layout();
                    item->setY(y);
                    y += item->boundingRect().height();
                }
            }

            QRectF StackItem::boundingRect() const
            {
                return QRectF(
                    0,
                    0,
                    _timeRange.duration().rescaled_to(1.0).value() * _zoom.x,
                    _options.margin + _options.fontLineSize + _options.margin + _tracksHeight());
            }

            void StackItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(95, 95, 95));
                painter->drawRect(
                    0,
                    0,
                    _timeRange.duration().rescaled_to(1.0).value() * _zoom.x,
                    _options.margin + _options.fontLineSize + _options.margin + _tracksHeight());

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _options.margin,
                    _options.margin + _options.fontLineSize - _options.fontDescender,
                    _label);
            }

            qreal StackItem::_tracksHeight() const
            {
                qreal out = 0;
                for (auto item : _trackItems)
                {
                    out += item->boundingRect().height();
                }
                return out;
            }
        }
    }
}
