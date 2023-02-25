// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "GapItem.h"

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            GapItem::GapItem(
                const otio::Gap* gap,
                const ItemData& itemData,
                QGraphicsItem* parent) :
                BaseItem(itemData, parent)
            {
                auto rangeOpt = gap->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _nameLabel(gap->name());
                _durationLabel = BaseItem::_durationLabel(gap->duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            void GapItem::setScale(float value)
            {
                if (value == _scale)
                    return;
                BaseItem::setScale(value);
                prepareGeometryChange();
            }

            void GapItem::setThumbnailHeight(int value)
            {
                if (value == _thumbnailHeight)
                    return;
                BaseItem::setThumbnailHeight(value);
                prepareGeometryChange();
            }

            QRectF GapItem::boundingRect() const
            {
                const math::Vector2f size = _size();
                return QRectF(0.F, 0.F, size.x, size.y);
            }

            void GapItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                const math::Vector2f size = _size();
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(60, 60, 90).lighter());
                painter->drawRect(0, 0, size.x, size.y);
                painter->setBrush(QColor(60, 60, 90));
                painter->drawRect(
                    _itemData.border,
                    _itemData.border,
                    size.x - _itemData.border - _itemData.border,
                    size.y - _itemData.border - _itemData.border);

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _itemData.border +
                    _itemData.margin,
                    _itemData.border +
                    _itemData.margin +
                    _itemData.fontYPos,
                    _label);
                painter->drawText(
                    _itemData.border +
                    _itemData.margin,
                    _itemData.border +
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontYPos,
                    _startLabel);

                QFontMetrics fm(_itemData.font);
                painter->drawText(
                    size.x -
                    _itemData.border -
                    _itemData.margin -
                    fm.width(_durationLabel),
                    _itemData.border +
                    _itemData.margin +
                    _itemData.fontYPos,
                    _durationLabel);
                painter->drawText(
                    size.x -
                    _itemData.border -
                    _itemData.margin -
                    fm.width(_endLabel),
                    _itemData.border +
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontYPos,
                    _endLabel);
            }

            QString GapItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    QString::fromUtf8(name.c_str()) :
                    QString("Gap");
            }

            math::Vector2f GapItem::_size() const
            {
                return math::Vector2f(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _itemData.border +
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.margin +
                    _itemData.border);
            }
        }
    }
}
