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
            struct ItemData
            {
                int margin = 5;
                int spacing = 5;
                int border = 1;

                QFont font;
                int fontLineSpacing = 0;
                int fontAscent = 0;
                int fontDescent = 0;
                int fontYPos = 0;

                int minTickSpacing = 5;
            };

            //! Base item.
            class BaseItem : public QGraphicsItem
            {
            public:
                BaseItem(
                    const ItemData&,
                    QGraphicsItem* parent = nullptr);

                virtual void setScale(float);

                virtual void setThumbnailHeight(int);

                virtual void layout();

            protected:
                static QString _durationLabel(const otime::RationalTime&);
                static QString _timeLabel(const otime::RationalTime&);

                ItemData _itemData;
                float _scale = 100.F;
                int _thumbnailHeight = 100;
            };
        }
    }
}
