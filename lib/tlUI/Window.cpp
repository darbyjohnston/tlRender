// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Window.h>

#include <tlUI/EventLoop.h>

namespace tl
{
    namespace ui
    {
        struct Window::Private
        {
            std::shared_ptr<observer::Value<bool> > open;
            std::shared_ptr<observer::Value<math::Size2i> > windowSize;
            std::shared_ptr<observer::Value<bool> > fullScreen;
            std::shared_ptr<observer::Value<bool> > floatOnTop;
        };

        void Window::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWindow::_init("tl::ui::Window", context, parent);
            TLRENDER_P();
            p.open = observer::Value<bool>::create(false);
            p.windowSize = observer::Value<math::Size2i>::create(math::Size2i(1280, 720));
            p.fullScreen = observer::Value<bool>::create(false);
            p.floatOnTop = observer::Value<bool>::create(false);
        }

        Window::Window() :
            _p(new Private)
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

        std::shared_ptr<observer::IValue<bool> > Window::observeOpen() const
        {
            return _p->open;
        }

        void Window::open(const std::shared_ptr<EventLoop>& eventLoop)
        {
            eventLoop->addWindow(
                std::dynamic_pointer_cast<IWindow>(shared_from_this()));
            _p->open->setIfChanged(true);
        }

        void Window::close()
        {
            if (auto eventLoop = _eventLoop.lock())
            {
                eventLoop->removeWindow(
                    std::dynamic_pointer_cast<IWindow>(shared_from_this()));
                _p->open->setIfChanged(false);
            }
        }

        std::shared_ptr<observer::IValue<math::Size2i> > Window::observeWindowSize() const
        {
            return _p->windowSize;
        }

        void Window::setWindowSize(const math::Size2i& value)
        {
            setGeometry(math::Box2i(_geometry.x(), _geometry.y(), value.w, value.h));
        }

        bool Window::isFullScreen() const
        {
            return _p->fullScreen->get();
        }

        std::shared_ptr<observer::IValue<bool> > Window::observeFullScreen() const
        {
            return _p->fullScreen;
        }

        void Window::setFullScreen(bool value)
        {
            _p->fullScreen->setIfChanged(value);
        }

        bool Window::isFloatOnTop() const
        {
            return _p->floatOnTop->get();
        }

        std::shared_ptr<observer::IValue<bool> > Window::observeFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void Window::setFloatOnTop(bool value)
        {
            _p->floatOnTop->setIfChanged(value);
        }

        void Window::setGeometry(const math::Box2i& value)
        {
            IWindow::setGeometry(value);
            for (const auto& i : _children)
            {
                i->setGeometry(value);
            }
            _p->windowSize->setIfChanged(value.getSize());
        }
    }
}
