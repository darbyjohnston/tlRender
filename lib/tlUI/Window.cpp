// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Window.h>

namespace tl
{
    namespace ui
    {
        void Window::_init(
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("tl::ui::Window", context);
        }

        Window::Window()
        {}

        Window::~Window()
        {}

        std::shared_ptr<Window> Window::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<Window>(new Window);
            out->_init(context);
            return out;
        }

        void Window::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            const math::BBox2i bbox(0, 0, value.w(), value.h());
            for (const auto& child : _children)
            {
                child->setGeometry(bbox);
            }
        }

        void Window::sizeHintEvent(const SizeHintEvent&)
        {
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
            }
        }

        void Window::drawEvent(const DrawEvent& event)
        {
            event.render->drawRect(
                _geometry,
                event.style->getColorRole(ColorRole::Window));
        }
    }
}
