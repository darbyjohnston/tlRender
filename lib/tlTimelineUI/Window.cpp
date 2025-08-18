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
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<feather_tk::App>& app,
            const std::string& name,
            const feather_tk::Size2I& size)
        {
            feather_tk::Window::_init(context, app, name, size);
        }

        Window::~Window()
        {}

        std::shared_ptr<Window> Window::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<feather_tk::App>& app,
            const std::string& name,
            const feather_tk::Size2I& size)
        {
            auto out = std::shared_ptr<Window>(new Window);
            out->_init(context, app, name, size);
            return out;
        }

        std::shared_ptr<feather_tk::IRender> Window::_createRender(const std::shared_ptr<feather_tk::Context>& context)
        {
            return timeline_gl::Render::create(context);
        }
    }
}
