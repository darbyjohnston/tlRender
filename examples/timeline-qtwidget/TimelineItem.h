// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <tlQt/TimelineThumbnailProvider.h>

#include <tlTimeline/Timeline.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Timeline item.
            class TimelineItem : public QObject, public BaseItem
            {
                Q_OBJECT

            public:
                TimelineItem(
                    const std::shared_ptr<timeline::Timeline>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    QGraphicsItem* parent = nullptr);

                ~TimelineItem() override;

                void setScale(float) override;
                void setThumbnailHeight(int) override;
                void layout() override;

                QRectF boundingRect() const override;
                void paint(
                    QPainter*,
                    const QStyleOptionGraphicsItem*,
                    QWidget* = nullptr) override;

            private Q_SLOTS:
                void _thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&);

            private:
                static QString _nameLabel(const std::string&);
                float _tracksHeight() const;
                math::Vector2f _size() const;

                std::shared_ptr<timeline::Timeline> _timeline;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::vector<BaseItem*> _trackItems;
                QString _label;
                QString _durationLabel;
                QString _startLabel;
                QString _endLabel;
                qt::TimelineThumbnailProvider* _thumbnailProvider = nullptr;
                qint64 _thumbnailRequestId = 0;
                QList<QPair<otime::RationalTime, QImage> > _thumbnails;
            };
        }
    }
}
