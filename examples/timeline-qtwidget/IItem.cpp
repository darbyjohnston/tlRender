// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "IItem.h"

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void IItem::_init(
                const std::string& name,
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(name, context, parent);
                _timeline = timeline;
            }

            IItem::IItem()
            {}

            IItem::~IItem()
            {}

            void IItem::setScale(float value)
            {
                if (value == _scale)
                    return;
                _setScale(
                    value,
                    std::dynamic_pointer_cast<IItem>(shared_from_this()));
            }

            void IItem::setThumbnailHeight(int value)
            {
                if (value == _thumbnailHeight)
                    return;
                _setThumbnailHeight(
                    value,
                    std::dynamic_pointer_cast<IItem>(shared_from_this()));
            }

            void IItem::setViewport(const math::BBox2i& value)
            {
                if (value == _viewport)
                    return;
                _setViewport(
                    value,
                    std::dynamic_pointer_cast<IItem>(shared_from_this()));
            }

            void IItem::_setScale(float value, const std::shared_ptr<IItem>& item)
            {
                item->_scale = value;
                item->_updates |= ui::Update::Size;
                item->_updates |= ui::Update::Draw;
                for (const auto& child : item->_children)
                {
                    if (auto item = std::dynamic_pointer_cast<IItem>(child))
                    {
                        _setScale(value, item);
                    }
                }
            }

            void IItem::_setThumbnailHeight(int value, const std::shared_ptr<IItem>& item)
            {
                item->_thumbnailHeight = value;
                item->_updates |= ui::Update::Size;
                item->_updates |= ui::Update::Draw;
                for (const auto& child : item->_children)
                {
                    if (auto item = std::dynamic_pointer_cast<IItem>(child))
                    {
                        _setThumbnailHeight(value, item);
                    }
                }
            }

            void IItem::_setViewport(const math::BBox2i& value, const std::shared_ptr<IItem>& item)
            {
                item->_viewport = value;
                item->_updates |= ui::Update::Size;
                item->_updates |= ui::Update::Draw;
                for (const auto& child : item->_children)
                {
                    if (auto item = std::dynamic_pointer_cast<IItem>(child))
                    {
                        _setViewport(value, item);
                    }
                }
            }

            std::string IItem::_durationLabel(const otime::RationalTime& value)
            {
                std::string out;
                if (value != time::invalidTime)
                {
                    out = string::Format("{0}@{1}").
                        arg(value.rescaled_to(1.0).value()).
                        arg(value.rate());
                }
                return out;
            }

            std::string IItem::_secondsLabel(const otime::RationalTime& value)
            {
                std::string out;
                if (value != time::invalidTime)
                {
                    out = string::Format("{0}").arg(value.rescaled_to(1.0).value());
                }
                return out;
            }

            std::string IItem::_frameLabel(const otime::RationalTime& value)
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
