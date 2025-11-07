// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineUI/Window.h>

#include <tlTimelineGL/Render.h>

namespace tl
{
    namespace timelineui
    {
        void Window::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::App>& app,
            const std::string& name,
            const ftk::Size2I& size)
        {
            ftk::Window::_init(context, app, name, size);
        }

        Window::~Window()
        {}

        std::shared_ptr<Window> Window::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::App>& app,
            const std::string& name,
            const ftk::Size2I& size)
        {
            auto out = std::shared_ptr<Window>(new Window);
            out->_init(context, app, name, size);
            return out;
        }

        std::shared_ptr<ftk::IRender> Window::_createRender(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            return timeline_gl::Render::create(logSystem);
        }
    }
}
