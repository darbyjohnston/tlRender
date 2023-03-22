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
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(name, context, parent);
            }

            IItem::IItem()
            {}

            IItem::~IItem()
            {}

            void IItem::setScale(float value)
            {
                if (value == _scale)
                    return;
                _scale = value;
                for (const auto& child : _children)
                {
                    if (auto item = std::dynamic_pointer_cast<IItem>(child))
                    {
                        item->setScale(value);
                    }
                }
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }

            void IItem::setThumbnailHeight(int value)
            {
                if (value == _thumbnailHeight)
                    return;
                _thumbnailHeight = value;
                for (const auto& child : _children)
                {
                    if (auto item = std::dynamic_pointer_cast<IItem>(child))
                    {
                        item->setThumbnailHeight(value);
                    }
                }
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }

            std::string IItem::_durationLabel(const otime::RationalTime& value)
            {
                std::string out;
                if (value != time::invalidTime)
                {
                    out = string::Format("{0}@{1}").arg(value.value()).arg(value.rate());
                }
                return out;
            }

            std::string IItem::_timeLabel(const otime::RationalTime& value)
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
