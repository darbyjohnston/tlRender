// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineItem.h"

#include "TrackItem.h"

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
                otime::RationalTime startTime(0.0, timeline->duration().rate());
                auto startTimeOpt = timeline->global_start_time();
                if (startTimeOpt.has_value())
                {
                    startTime = startTimeOpt.value().rescaled_to(startTime.value());
                }
                _timeRange = otime::TimeRange(startTime, timeline->duration());

                for (const auto& child : timeline->tracks()->children())
                {
                    if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                    {
                        auto trackItem = new TrackItem(track, options);
                        trackItem->setParentItem(this);
                        _trackItems.push_back(trackItem);
                    }
                }

                _label = _nameLabel(timeline->name());
                _durationLabel = BaseItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            void TimelineItem::layout()
            {
                qreal y = _options.margin + _options.fontLineSize + _options.spacing +
                    _options.fontLineSize + _options.margin;
                for (auto item : _trackItems)
                {
                    item->layout();
                    item->setY(y);
                    y += item->boundingRect().height();
                }
            }

            QRectF TimelineItem::boundingRect() const
            {
                return QRectF(
                    0,
                    0,
                    _timeRange.duration().rescaled_to(1.0).value() * _zoom.x,
                    _options.margin + _options.fontLineSize + _options.spacing +
                        _options.fontLineSize + _options.margin +
                        _tracksHeight());
            }

            void TimelineItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                const float w = _timeRange.duration().rescaled_to(1.0).value() * _zoom.x;
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(63, 63, 63));
                painter->drawRect(
                    0,
                    0,
                    w,
                    _options.margin + _options.fontLineSize + _options.spacing +
                        _options.fontLineSize + _options.margin +
                        _tracksHeight());

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

            QString TimelineItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    QString::fromUtf8(name.c_str()) :
                    QString("Timeline");
            }

            qreal TimelineItem::_tracksHeight() const
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
