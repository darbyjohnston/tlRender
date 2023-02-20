// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <opentimelineio/gap.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Gap item.
            class GapItem : public BaseItem
            {
            public:
                GapItem(
                    const otio::Gap*,
                    const ItemOptions&,
                    QGraphicsItem* parent = nullptr);

                QRectF boundingRect() const override;
                void paint(
                    QPainter*,
                    const QStyleOptionGraphicsItem*,
                    QWidget* = nullptr) override;

            private:
                QString _label;
                otime::TimeRange _timeRange;
            };
        }
    }
}
