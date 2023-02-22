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

            void ClipItem::setScale(float value)
            {
                if (value == _scale)
                    return;
                BaseItem::setScale(value);
                prepareGeometryChange();
            }

            void ClipItem::setThumbnailHeight(int value)
            {
                if (value == _thumbnailHeight)
                    return;
                BaseItem::setThumbnailHeight(value);
                prepareGeometryChange();
            }

            QRectF ClipItem::boundingRect() const
            {
                const math::Vector2f size = _size();
                return QRectF(0.F, 0.F, size.x, size.y);
            }

            void ClipItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                const math::Vector2f size = _size();
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(60, 90, 60));
                painter->drawRect(0.F, 0.F, size.x, size.y);

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _options.margin,
                    _options.margin +
                    _options.fontLineSize - _options.fontDescender,
                    _label);
                painter->drawText(
                    _options.margin,
                    _options.margin +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize - _options.fontDescender,
                    _startLabel);

                QFontMetrics fm(_options.font);
                painter->drawText(
                    size.x - _options.margin - fm.width(_durationLabel),
                    _options.margin +
                    _options.fontLineSize - _options.fontDescender,
                    _durationLabel);
                painter->drawText(
                    size.x - _options.margin - fm.width(_endLabel),
                    _options.margin +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize - _options.fontDescender,
                    _endLabel);
            }

            QString ClipItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    QString::fromUtf8(name.c_str()) :
                    QString("Clip");
            }

            math::Vector2f ClipItem::_size() const
            {
                return math::Vector2f(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _options.margin +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize +
                    _options.margin);
            }
        }
    }
}
