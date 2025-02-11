// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/Window.h>

#include <tlTimelineGL/Render.h>

namespace tl
{
    namespace timelineui
    {
        void Window::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& name,
            const dtk::Size2I& size)
        {
            dtk::Window::_init(context, name, size);
        }

        Window::~Window()
        {}

        std::shared_ptr<Window> Window::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& name,
            const dtk::Size2I& size)
        {
            auto out = std::shared_ptr<Window>(new Window);
            out->_init(context, name, size);
            return out;
        }

        std::shared_ptr<dtk::IRender> Window::_createRender(const std::shared_ptr<dtk::Context>& context)
        {
            return timeline_gl::Render::create(context);
        }
    }
}
