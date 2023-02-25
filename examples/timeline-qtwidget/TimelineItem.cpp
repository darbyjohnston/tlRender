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
                const std::shared_ptr<timeline::Timeline>& timeline,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                QGraphicsItem* parent) :
                BaseItem(itemData, parent),
                _timeline(timeline)
            {
                _timeRange = timeline->getTimeRange();

                const auto otioTimeline = timeline->getTimeline();
                for (const auto& child : otioTimeline->tracks()->children())
                {
                    if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                    {
                        auto trackItem = new TrackItem(track, itemData);
                        trackItem->setParentItem(this);
                        _trackItems.push_back(trackItem);
                    }
                }

                _label = _nameLabel(otioTimeline->name());
                _durationLabel = BaseItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());

                _thumbnailProvider = new qt::TimelineThumbnailProvider(context);
                connect(
                    _thumbnailProvider,
                    SIGNAL(thumbails(qint64, const QList<QPair<otime::RationalTime, QImage> >&)),
                    SLOT(_thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&)));
            }

            TimelineItem::~TimelineItem()
            {
                delete _thumbnailProvider;
            }

            void TimelineItem::setScale(float value)
            {
                if (value == _scale)
                    return;
                BaseItem::setScale(value);
                prepareGeometryChange();
                for (auto trackItem : _trackItems)
                {
                    trackItem->setScale(value);
                }
                layout();
            }

            void TimelineItem::setThumbnailHeight(int value)
            {
                if (value == _thumbnailHeight)
                    return;
                BaseItem::setThumbnailHeight(value);
                prepareGeometryChange();
                for (auto trackItem : _trackItems)
                {
                    trackItem->setThumbnailHeight(value);
                }
                layout();
            }

            void TimelineItem::layout()
            {
                const math::Vector2f size = _size();
                float y =
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _thumbnailHeight;
                for (auto item : _trackItems)
                {
                    item->layout();
                    item->setPos(_itemData.margin, y);
                    y += item->boundingRect().height();
                }
                
                _thumbnails.clear();
                _thumbnailProvider->cancelRequests(_thumbnailRequestId);
                const auto ioInfo = _timeline->getIOInfo();
                const int thumbnailWidth = !ioInfo.video.empty() ?
                    static_cast<int>(_thumbnailHeight * ioInfo.video[0].size.getAspect()) :
                    0;
                QList<otime::RationalTime> thumbnailTimes;
                for (float x = _itemData.margin; x < size.x - _itemData.margin * 2; x += thumbnailWidth)
                {
                    thumbnailTimes.push_back(otime::RationalTime(
                        _timeRange.start_time().value() + (x - _itemData.margin) / (size.x - _itemData.margin * 2) * _timeRange.duration().value(),
                        _timeRange.duration().rate()));
                }
                _thumbnailRequestId = _thumbnailProvider->request(
                    QString::fromUtf8(_timeline->getPath().get().c_str()),
                    QSize(thumbnailWidth, _thumbnailHeight),
                    thumbnailTimes);
            }

            QRectF TimelineItem::boundingRect() const
            {
                const math::Vector2f size = _size();
                return QRectF(0.F, 0.F, size.x, size.y);
            }

            void TimelineItem::paint(
                QPainter* painter,
                const QStyleOptionGraphicsItem*,
                QWidget*)
            {
                const math::Vector2f size = _size();
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(40, 40, 40));
                painter->drawRect(0.F, 0.F, size.x, size.y);

                painter->setPen(QColor(240, 240, 240));
                painter->drawText(
                    _itemData.margin,
                    _itemData.margin +
                    _itemData.fontYPos,
                    _label);
                painter->drawText(
                    _itemData.margin,
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontYPos,
                    _startLabel);

                QFontMetrics fm(_itemData.font);
                painter->drawText(
                    size.x -
                    _itemData.margin -
                    fm.width(_durationLabel),
                    _itemData.margin +
                    _itemData.fontYPos,
                    _durationLabel);
                painter->drawText(
                    size.x -
                    _itemData.margin -
                    fm.width(_endLabel),
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontYPos,
                    _endLabel);

                const float frameTick0 = _timeRange.start_time().value() /
                    _timeRange.duration().value() * (size.x - _itemData.margin * 2);
                const float frameTick1 = (_timeRange.start_time().value() + 1.0) /
                    _timeRange.duration().value() * (size.x - _itemData.margin * 2);
                const int frameWidth = frameTick1 - frameTick0;
                if (frameWidth >= _itemData.minTickSpacing)
                {
                    QString label = QString("%1").arg(_timeRange.end_time_inclusive().value());
                    if (fm.width(label) < (frameWidth - _itemData.spacing))
                    {
                        painter->setPen(QColor(120, 120, 120));
                        for (double t = 1.0; t < _timeRange.duration().value(); t += 1.0)
                        {
                            label = QString("%1").arg(t);
                            painter->drawText(
                                _itemData.margin + t / _timeRange.duration().value() * (size.x - _itemData.margin * 2),
                                _itemData.margin +
                                _itemData.fontLineSpacing +
                                _itemData.spacing +
                                _itemData.fontLineSpacing +
                                _itemData.spacing +
                                _itemData.fontYPos,
                                label);
                        }
                    }

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QColor(80, 80, 80));
                    for (double t = 1.0; t < _timeRange.duration().value(); t += 1.0)
                    {
                        painter->drawRect(
                            _itemData.margin + t / _timeRange.duration().value() * (size.x - _itemData.margin * 2),
                            _itemData.margin +
                            _itemData.fontLineSpacing +
                            _itemData.spacing +
                            _itemData.fontLineSpacing +
                            _itemData.spacing +
                            _itemData.fontLineSpacing +
                            _itemData.spacing,
                            1,
                            size.y -
                            _itemData.margin -
                            _itemData.fontLineSpacing -
                            _itemData.spacing -
                            _itemData.fontLineSpacing -
                            _itemData.spacing -
                            _itemData.fontLineSpacing -
                            _itemData.spacing -
                            _itemData.margin);
                    }
                }

                const float secondsTick0 = _timeRange.start_time().value() /
                    (_timeRange.duration().value() / _timeRange.duration().rate()) * (size.x - _itemData.margin * 2);
                const float secondsTick1 = (_timeRange.start_time().value() + 1.0) /
                    (_timeRange.duration().value() / _timeRange.duration().rate()) * (size.x - _itemData.margin * 2);
                const int secondsWidth = secondsTick1 - secondsTick0;
                if (secondsWidth >= _itemData.minTickSpacing)
                {
                    QString label = QString("%1").arg(_timeRange.end_time_inclusive().value());
                    if (fm.width(label) < (secondsWidth - _itemData.spacing))
                    {
                        painter->setPen(QColor(240, 240, 240));
                        for (double t = 0.0;
                            t < _timeRange.duration().value();
                            t += _timeRange.duration().rate())
                        {
                            label = QString("%1").arg(t);
                            painter->drawText(
                                _itemData.margin + t / _timeRange.duration().value() * (size.x - _itemData.margin * 2),
                                _itemData.margin +
                                _itemData.fontLineSpacing +
                                _itemData.spacing +
                                _itemData.fontLineSpacing +
                                _itemData.spacing +
                                _itemData.fontYPos,
                                label);
                        }
                    }

                    painter->setPen(Qt::NoPen);
                    painter->setBrush(QColor(160, 160, 160));
                    for (double t = 0.0;
                        t < _timeRange.duration().value();
                        t += _timeRange.duration().rate())
                    {
                        painter->drawRect(
                            _itemData.margin + t / _timeRange.duration().value() * (size.x - _itemData.margin * 2),
                            _itemData.margin +
                            _itemData.fontLineSpacing +
                            _itemData.spacing +
                            _itemData.fontLineSpacing +
                            _itemData.spacing +
                            _itemData.fontLineSpacing +
                            _itemData.spacing,
                            1,
                            size.y -
                            _itemData.margin -
                            _itemData.fontLineSpacing -
                            _itemData.spacing -
                            _itemData.fontLineSpacing -
                            _itemData.spacing -
                            _itemData.fontLineSpacing -
                            _itemData.spacing -
                            _itemData.margin);
                    }
                }

                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(0, 0, 0));
                painter->drawRect(
                    _itemData.margin,
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing,
                    size.x - _itemData.margin * 2,
                    _thumbnailHeight);
                painter->setClipRect(_itemData.margin, 0, size.x - _itemData.margin * 2, size.y);
                for (const auto& thumbnail : _thumbnails)
                {
                    painter->drawImage(
                        _itemData.margin +
                        (thumbnail.first.value() - _timeRange.start_time().value()) /
                            _timeRange.duration().value() * (size.x - _itemData.margin * 2),
                        _itemData.margin +
                        _itemData.fontLineSpacing +
                        _itemData.spacing +
                        _itemData.fontLineSpacing +
                        _itemData.spacing +
                        _itemData.fontLineSpacing +
                        _itemData.spacing +
                        _itemData.fontLineSpacing +
                        _itemData.spacing,
                        thumbnail.second);
                }
            }

            void TimelineItem::_thumbnailsCallback(qint64 id, const QList<QPair<otime::RationalTime, QImage> >& thumbnails)
            {
                if (_thumbnailRequestId == id)
                {
                    _thumbnails.append(thumbnails);
                    update();
                }
            }

            QString TimelineItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    QString::fromUtf8(name.c_str()) :
                    QString("Timeline");
            }

            float TimelineItem::_tracksHeight() const
            {
                float out = 0.F;
                for (auto item : _trackItems)
                {
                    out += item->boundingRect().height();
                }
                return out;
            }

            math::Vector2f TimelineItem::_size() const
            {
                return math::Vector2f(
                    _itemData.margin +
                    _timeRange.duration().rescaled_to(1.0).value() * _scale +
                    _itemData.margin,
                    _itemData.margin +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _itemData.fontLineSpacing +
                    _itemData.spacing +
                    _thumbnailHeight +
                    _tracksHeight() +
                    _itemData.margin);
            }
        }
    }
}
