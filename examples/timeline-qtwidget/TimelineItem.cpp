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
                const ItemOptions& options,
                const std::shared_ptr<system::Context>& context,
                QGraphicsItem* parent) :
                BaseItem(options, parent),
                _timeline(timeline)
            {
                _timeRange = timeline->getTimeRange();

                const auto otioTimeline = timeline->getTimeline();
                for (const auto& child : otioTimeline->tracks()->children())
                {
                    if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                    {
                        auto trackItem = new TrackItem(track, options);
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
                    _options.margin +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize +
                    _options.spacing +
                    _thumbnailHeight;
                for (auto item : _trackItems)
                {
                    item->layout();
                    item->setY(y);
                    y += item->boundingRect().height();
                }
                
                _thumbnails.clear();
                _thumbnailProvider->cancelRequests(_thumbnailRequestId);
                const auto ioInfo = _timeline->getIOInfo();
                const int thumbnailWidth = !ioInfo.video.empty() ?
                    static_cast<int>(_thumbnailHeight * ioInfo.video[0].size.getAspect()) :
                    0;
                QList<otime::RationalTime> thumbnailTimes;
                for (float x = 0.F; x < size.x; x += thumbnailWidth)
                {
                    thumbnailTimes.push_back(otime::RationalTime(
                        _timeRange.start_time().value() + x / size.x * _timeRange.duration().value(),
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

                painter->setClipRect(0, 0, size.x, size.y);
                for (const auto& thumbnail : _thumbnails)
                {
                    painter->drawImage(
                        (thumbnail.first.value() - _timeRange.start_time().value()) / _timeRange.duration().value() * size.x,
                        _options.margin +
                        _options.fontLineSize +
                        _options.spacing +
                        _options.fontLineSize +
                        _options.spacing +
                        _options.fontLineSize +
                        _options.spacing +
                        _options.fontLineSize +
                        _options.spacing,
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
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _options.margin +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize +
                    _options.spacing +
                    _options.fontLineSize +
                    _options.spacing +
                    _thumbnailHeight +
                    _tracksHeight());
            }
        }
    }
}
