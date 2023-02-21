// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <opentimelineio/clip.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Clip item.
            class ClipItem : public BaseItem
            {
            public:
                ClipItem(
                    const otio::Clip*,
                    const ItemOptions&,
                    QGraphicsItem* parent = nullptr);

                QRectF boundingRect() const override;
                void paint(
                    QPainter*,
                    const QStyleOptionGraphicsItem*,
                    QWidget* = nullptr) override;

            private:
                static QString _nameLabel(const std::string&);
                math::Vector2f _size() const;

                otime::TimeRange _timeRange = time::invalidTimeRange;
                QString _label;
                QString _durationLabel;
                QString _startLabel;
                QString _endLabel;
            };
        }
    }
}
