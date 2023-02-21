// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <QGraphicsItem>
#include <QFont>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            struct ItemOptions
            {
                int margin = 5;
                int spacing = 5;
                int border = 2;
                QFont font;
                int fontLineSize = 0;
                int fontAscender = 0;
                int fontDescender = 0;
                int thumbnailHeight = 100;
            };

            //! Base item.
            class BaseItem : public QGraphicsItem
            {
            public:
                BaseItem(
                    const ItemOptions&,
                    QGraphicsItem* parent = nullptr);

                virtual void setZoom(const math::Vector2f&);

                virtual void layout();

            protected:
                static QString _durationLabel(const otime::RationalTime&);
                static QString _timeLabel(const otime::RationalTime&);

                ItemOptions _options;
                math::Vector2f _zoom = math::Vector2f(100.F, 1.F);
            };
        }
    }
}
