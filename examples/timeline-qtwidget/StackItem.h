// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <opentimelineio/stack.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Stack item.
            class StackItem : public BaseItem
            {
            public:
                StackItem(
                    const otio::Stack*,
                    const ItemOptions&,
                    QGraphicsItem* parent = nullptr);

                void layout() override;

                QRectF boundingRect() const override;
                void paint(
                    QPainter*,
                    const QStyleOptionGraphicsItem*,
                    QWidget* = nullptr) override;

            private:
                qreal _tracksHeight() const;

                QString _label;
                otime::TimeRange _timeRange;
                std::vector<BaseItem*> _trackItems;
            };
        }
    }
}
