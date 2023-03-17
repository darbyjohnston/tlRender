// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Window.h>

namespace tl
{
    namespace ui
    {
        void Window::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, parent);
        }

        Window::Window()
        {}

        Window::~Window()
        {}

        std::shared_ptr<Window> Window::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Window>(new Window);
            out->_init(context, parent);
            return out;
        }

        void Window::sizeHint(const SizeHintData& data)
        {
            IWidget::sizeHint(data);
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
            }
        }

        void Window::setGeometry(const math::BBox2i& value)
        {
            const math::BBox2i bbox(0, 0, value.w(), value.h());
            for (const auto& child : _children)
            {
                child->setGeometry(bbox);
            }
        }

        void Window::draw(const DrawData& data)
        {
            data.render->drawRect(
                data.bbox,
                data.style->getColorRole(ColorRole::Window));
            IWidget::draw(data);
        }
    }
}
