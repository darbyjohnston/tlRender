// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "BaseItem.h"

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void BaseItem::_init(
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                _itemData = itemData;
                _context = context;
            }

            BaseItem::BaseItem()
            {}

            BaseItem::~BaseItem()
            {}

            void BaseItem::setScale(float value)
            {
                if (value == _scale)
                    return;
                _scale = value;
                for (const auto& child : _children)
                {
                    child->setScale(value);
                }
                _doLayout = true;
                _doRender = true;
            }

            void BaseItem::setThumbnailHeight(int value)
            {
                if (value == _thumbnailHeight)
                    return;
                _thumbnailHeight = value;
                for (const auto& child : _children)
                {
                    child->setThumbnailHeight(value);
                }
                _doLayout = true;
                _doRender = true;
            }

            const std::list<std::shared_ptr<BaseItem> >& BaseItem::children() const
            {
                return _children;
            }

            bool BaseItem::doLayout() const
            {
                return _doLayout;
            }
            
            void BaseItem::preLayout()
            {}

            math::Vector2i BaseItem::sizeHint() const
            {
                return _sizeHint;
            }

            void BaseItem::layout(const math::BBox2i& value)
            {
                _doLayout = false;
                _geometry = value;
            }

            bool BaseItem::doRender() const
            {
                return _doRender;
            }

            void BaseItem::render(
                const std::shared_ptr<timeline::IRender>&,
                const math::BBox2i&,
                float)
            {
                _doRender = false;
            }

            void BaseItem::tick()
            {}

            std::string BaseItem::_durationLabel(const otime::RationalTime& value)
            {
                std::string out;
                if (value != time::invalidTime)
                {
                    out = string::Format("{0}@{1}").arg(value.value()).arg(value.rate());
                }
                return out;
            }

            std::string BaseItem::_timeLabel(const otime::RationalTime& value)
            {
                std::string out;
                if (value != time::invalidTime)
                {
                    out = string::Format("{0}").arg(value.value());
                }
                return out;
            }
        }
    }
}
