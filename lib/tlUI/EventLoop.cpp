// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/EventLoop.h>

namespace tl
{
    namespace ui
    {
        struct EventLoop::Private
        {
            std::weak_ptr<system::Context> context;
            std::list<std::weak_ptr<Window> > windows;
        };

        void EventLoop::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
        }

        EventLoop::EventLoop() :
            _p(new Private)
        {}

        EventLoop::~EventLoop()
        {}

        std::shared_ptr<EventLoop> EventLoop::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<EventLoop>(new EventLoop);
            out->_init(context);
            return out;
        }

        void EventLoop::addWindow(const std::weak_ptr<Window>& window)
        {
            TLRENDER_P();
            p.windows.push_back(window);
        }

        void EventLoop::tick()
        {

        }

        std::chrono::milliseconds EventLoop::getTickTime() const
        {
            return std::chrono::milliseconds(10);
        }
    }
}
