// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/StackLayout.h>

namespace tl
{
    namespace ui
    {
        void StackLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::StackLayout", context, parent);
        }

        StackLayout::StackLayout()
        {}

        StackLayout::~StackLayout()
        {}

        std::shared_ptr<StackLayout> StackLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StackLayout>(new StackLayout);
            out->_init(context, parent);
            return out;
        }

        void StackLayout::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            for (const auto& child : _children)
            {
                child->setGeometry(value);
            }
        }

        void StackLayout::sizeHintEvent(const SizeHintEvent&)
        {
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
            }
        }
    }
}
