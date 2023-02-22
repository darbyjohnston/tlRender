// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TrackItem.h"

#include "ClipItem.h"
#include "GapItem.h"

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            TrackItem::TrackItem(
                const otio::Track* track,
                const ItemOptions& options,
                QGraphicsItem* parent) :
                BaseItem(options, parent)
            {
                _timeRange = track->trimmed_range();

                for (auto child : track->children())
                {
                    if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                    {
                        auto clipItem = new ClipItem(clip, _options);
                        clipItem->setParentItem(this);
                        _items.push_back(clipItem);
                        const auto timeRangeOpt = track->trimmed_range_of_child(clip);
                        if (timeRangeOpt.has_value())
                        {
                            _timeRanges[clipItem] = timeRangeOpt.value();
                        }
                    }
                    else if (auto gap = dynamic_cast<otio::Gap*>(child.value))
                    {
                        auto gapItem = new GapItem(gap, _options);
                        gapItem->setParentItem(this);
                        _items.push_back(gapItem);
                        const auto timeRangeOpt = track->trimmed_range_of_child(gap);
                        if (timeRangeOpt.has_value())
                        {
                            _timeRanges[gapItem] = timeRangeOpt.value();
                        }
                    }

                    _label = _nameLabel(track->kind(), track->name());
                    _durationLabel = BaseItem::_durationLabel(_timeRange.duration());
                }
            }

            void TrackItem::setScale(float value)
            {
                if (value == _scale)
                    return;
                BaseItem::setScale(value);
                prepareGeometryChange();
                for (auto item : _items)
                {
                    item->setScale(value);
                }
            }

            void TrackItem::setThumbnailHeight(int value)
            {
                if (value == _thumbnailHeight)
                    return;
                BaseItem::setThumbnailHeight(value);
                prepareGeometryChange();
                for (auto item : _items)
                {
                    item->setThumbnailHeight(value);
                }
            }

            void TrackItem::layout()
            {
                for (auto item : _items)
                {
                    item->layout();
                    const auto i = _timeRanges.find(item);
                    if (i != _timeRanges.end())
                    {
                        item->setPos(
                            i->second.start_time().rescaled_to(1.0).value() * _scale,
                            _options.margin +
                            _options.fontLineSize +
                            _options.margin);
                    }
                }
            }

            QRectF TrackItem::boundingRect() const
            {
                const math::Vector2f size = _size();
                return QRectF(0.F, 0.F, size.x, size.y);
            }

            void TrackItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                const math::Vector2f size = _size();
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(60, 60, 60));
                painter->drawRect(0.F, 0.F, size.x, size.y);

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _options.margin,
                    _options.margin + _options.fontLineSize - _options.fontDescender,
                    _label);

                QFontMetrics fm(_options.font);
                painter->drawText(
                    size.x - _options.margin - fm.width(_durationLabel),
                    _options.margin +
                    _options.fontLineSize - _options.fontDescender,
                    _durationLabel);
            }

            QString TrackItem::_nameLabel(
                const std::string& kind,
                const std::string& name)
            {
                return !name.empty() && name != "Track" ?
                    QString("%1 Track: %2").
                        arg(QString::fromUtf8(kind.c_str())).
                        arg(QString::fromUtf8(name.c_str())) :
                    QString("%1 Track").
                        arg(QString::fromUtf8(kind.c_str()));
            }

            float TrackItem::_itemsHeight() const
            {
                float out = 0.F;
                for (auto item : _items)
                {
                    out = std::max(out, static_cast<float>(item->boundingRect().height()));
                }
                return out;
            }

            math::Vector2f TrackItem::_size() const
            {
                return math::Vector2f(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _options.margin +
                    _options.fontLineSize +
                    _options.margin +
                    _itemsHeight());
            }
        }
    }
}
