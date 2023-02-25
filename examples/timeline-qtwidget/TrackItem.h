// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <opentimelineio/track.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Track item.
            class TrackItem : public BaseItem
            {
            public:
                TrackItem(
                    const otio::Track*,
                    const ItemData&,
                    QGraphicsItem* parent = nullptr);

                void setScale(float) override;
                void setThumbnailHeight(int) override;
                void layout() override;

                QRectF boundingRect() const override;
                void paint(
                    QPainter*,
                    const QStyleOptionGraphicsItem*,
                    QWidget* = nullptr) override;

            private:
                static QString _nameLabel(
                    const std::string& kind,
                    const std::string& name);

                float _itemsHeight() const;
                math::Vector2f _size() const;

                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::vector<BaseItem*> _items;
                std::map<BaseItem*, otime::TimeRange> _timeRanges;
                QString _label;
                QString _durationLabel;
            };
        }
    }
}
