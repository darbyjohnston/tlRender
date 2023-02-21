// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "BaseItem.h"

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            BaseItem::BaseItem(
                const ItemOptions& options,
                QGraphicsItem* parent) :
                QGraphicsItem(parent),
                _options(options)
            {}

            void BaseItem::setZoom(const tl::math::Vector2f& value)
            {
                if (value == _zoom)
                    return;
                _zoom = value;
            }

            void BaseItem::layout()
            {}

            QString BaseItem::_durationLabel(const otime::RationalTime& value)
            {
                return value != time::invalidTime ?
                    QString("%1@%2").arg(value.value()).arg(value.rate()) :
                    QString();
            }

            QString BaseItem::_timeLabel(const otime::RationalTime& value)
            {
                return value != time::invalidTime ?
                    QString("%1").arg(value.value()) :
                    QString();
            }
        }
    }
}
