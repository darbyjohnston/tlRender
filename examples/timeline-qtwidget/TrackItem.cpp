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

            void TrackItem::layout()
            {
                for (auto item : _items)
                {
                    item->layout();
                    const auto i = _timeRanges.find(item);
                    if (i != _timeRanges.end())
                    {
                        item->setPos(
                            i->second.start_time().rescaled_to(1.0).value() * _zoom.x,
                            _options.margin + _options.fontLineSize + _options.margin);
                    }
                }
            }

            QRectF TrackItem::boundingRect() const
            {
                return QRectF(
                    0,
                    0,
                    _timeRange.duration().rescaled_to(1.0).value() * _zoom.x,
                    _options.margin + _options.fontLineSize + _options.margin +
                        _itemsHeight());
            }

            void TrackItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                const float w = _timeRange.duration().rescaled_to(1.0).value() * _zoom.x;
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(127, 127, 127));
                painter->drawRect(
                    0,
                    0,
                    w,
                    _options.margin + _options.fontLineSize + _options.margin + _itemsHeight());

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _options.margin,
                    _options.margin + _options.fontLineSize - _options.fontDescender,
                    _label);

                QFontMetrics fm(_options.font);
                painter->drawText(
                    w - _options.margin - fm.width(_durationLabel),
                    _options.margin + _options.fontLineSize - _options.fontDescender,
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

            qreal TrackItem::_itemsHeight() const
            {
                qreal out = 0;
                for (auto item : _items)
                {
                    out = std::max(out, item->boundingRect().height());
                }
                return out;
            }
        }
    }
}
