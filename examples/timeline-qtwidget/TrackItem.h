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
                    const ItemOptions&,
                    QGraphicsItem* parent = nullptr);

                void layout() override;

                QRectF boundingRect() const override;
                void paint(
                    QPainter*,
                    const QStyleOptionGraphicsItem*,
                    QWidget* = nullptr) override;

            private:
                qreal _itemsHeight() const;

                QString _label;
                otime::TimeRange _timeRange;
                std::vector<BaseItem*> _items;
                std::map<BaseItem*, otime::TimeRange> _timeRanges;
            };
        }
    }
}
