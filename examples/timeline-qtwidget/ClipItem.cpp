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
                auto rangeOpt = clip->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _nameLabel(clip->name());
                _durationLabel = BaseItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            QRectF ClipItem::boundingRect() const
            {
                return QRectF(
                    0,
                    0,
                    _timeRange.duration().rescaled_to(1.0).value() * _zoom.x,
                    _options.margin + _options.fontLineSize + _options.spacing +
                        _options.fontLineSize + _options.margin +
                        _options.thumbnailHeight * _zoom.y);
            }

            void ClipItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                const float w = _timeRange.duration().rescaled_to(1.0).value() * _zoom.x;
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(63, 127, 63));
                painter->drawRect(
                    0,
                    0,
                    w,
                    _options.margin + _options.fontLineSize + _options.spacing +
                        _options.fontLineSize + _options.margin +
                        _options.thumbnailHeight * _zoom.y);

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _options.margin,
                    _options.margin + _options.fontLineSize - _options.fontDescender,
                    _label);
                painter->drawText(
                    _options.margin,
                    _options.margin + _options.fontLineSize + _options.spacing +
                        _options.fontLineSize - _options.fontDescender,
                    _startLabel);

                QFontMetrics fm(_options.font);
                painter->drawText(
                    w - _options.margin - fm.width(_durationLabel),
                    _options.margin + _options.fontLineSize - _options.fontDescender,
                    _durationLabel);
                painter->drawText(
                    w - _options.margin - fm.width(_endLabel),
                    _options.margin + _options.fontLineSize + _options.spacing +
                        _options.fontLineSize - _options.fontDescender,
                    _endLabel);
            }

            QString ClipItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    QString::fromUtf8(name.c_str()) :
                    QString("Clip");
            }
        }
    }
}
