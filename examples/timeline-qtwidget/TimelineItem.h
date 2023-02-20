// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <opentimelineio/timeline.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Timeline item.
            class TimelineItem : public BaseItem
            {
            public:
                TimelineItem(
                    const otio::Timeline*,
                    const ItemOptions&,
                    QGraphicsItem* parent = nullptr);

                void layout() override;

                QRectF boundingRect() const override;
                void paint(
                    QPainter*,
                    const QStyleOptionGraphicsItem*,
                    QWidget* = nullptr) override;

            private:
                QString _label;
                otime::RationalTime _duration;
                BaseItem* _stackItem = nullptr;
            };
        }
    }
}
