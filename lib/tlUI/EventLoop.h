// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Window.h>

#include <tlCore/ISystem.h>

namespace tl
{
    //! User interface.
    namespace ui
    {
        //! Event loop.
        class EventLoop : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(EventLoop);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            EventLoop();

        public:
            ~EventLoop() override;

            //! Create a new event loop.
            static std::shared_ptr<EventLoop> create(
                const std::shared_ptr<system::Context>&);

            //! Add a window to the event loop.
            void addWindow(const std::weak_ptr<Window>&);

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
