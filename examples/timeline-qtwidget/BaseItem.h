// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <tlCore/FontSystem.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Item data.
            struct ItemData
            {
                int margin = 5;
                int spacing = 5;
                int border = 1;

                std::shared_ptr<imaging::FontSystem> fontSystem;
                imaging::FontInfo fontInfo;
                imaging::FontMetrics fontMetrics;

                int minTickSpacing = 5;
            };

            //! Base item.
            class BaseItem : public std::enable_shared_from_this<BaseItem>
            {
            protected:
                void _init(
                    const ItemData&,
                    const std::shared_ptr<system::Context>&);

                BaseItem();

            public:
                virtual ~BaseItem() = 0;

                virtual void setScale(float);

                virtual void setThumbnailHeight(int);

                const std::list<std::shared_ptr<BaseItem> >& children() const;

                bool doLayout() const;

                virtual void preLayout();

                math::Vector2i sizeHint() const;

                virtual void layout(const math::BBox2i&);

                bool doRender() const;

                virtual void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const math::BBox2i& viewport,
                    float devicePixelRatio);

                virtual void tick();

            protected:
                static std::string _durationLabel(const otime::RationalTime&);
                static std::string _timeLabel(const otime::RationalTime&);

                std::weak_ptr<system::Context> _context;
                ItemData _itemData;
                float _scale = 100.F;
                int _thumbnailHeight = 100;
                std::list<std::shared_ptr<BaseItem> > _children;
                bool _doLayout = true;
                math::Vector2i _sizeHint;
                math::BBox2i _geometry;
                bool _doRender = true;
            };
        }
    }
}
