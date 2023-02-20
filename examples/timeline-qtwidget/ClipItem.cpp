// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ClipItem.h"

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            ClipItem::ClipItem(
                const otio::Clip* clip,
                const ItemOptions& options,
                QGraphicsItem* parent) :
                BaseItem(options, parent)
            {
                _label = QString("Clip: %1").arg(QString::fromUtf8(clip->name().c_str()));

                _timeRange = clip->trimmed_range();
            }

            QRectF ClipItem::boundingRect() const
            {
                return QRectF(
                    0,
                    0,
                    _timeRange.duration().rescaled_to(1.0).value() * _zoom.x,
                    (_options.fontLineSize + _options.margin * 2.F + _options.thumbnailHeight) * _zoom.y);
            }

            void ClipItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(63, 127, 63));
                painter->drawRect(
                    0,
                    0,
                    _timeRange.duration().rescaled_to(1.0).value() * _zoom.x,
                    (_options.fontLineSize + _options.margin * 2.F + _options.thumbnailHeight) * _zoom.y);

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _options.margin,
                    _options.margin + _options.fontLineSize - _options.fontDescender,
                    _label);
            }
        }
    }
}
