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
            void _init(
                const std::shared_ptr<Style>&,
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<system::Context>&);

            EventLoop();

        public:
            ~EventLoop() override;

            //! Create a new event loop.
            static std::shared_ptr<EventLoop> create(
                const std::shared_ptr<Style>&,
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<system::Context>&);

            //! Set the frame buffer size.
            void setFrameBufferSize(const imaging::Size&);

            //! Set the content scale.
            void setContentScale(float);

            //! Key.
            void key(Key, bool press);

            //! Cursor enter.
            void cursorEnter(bool enter);

            //! Cursor position.
            void cursorPos(const math::Vector2i&);

            //! Mouse button.
            void mouseButton(int button, bool press, int modifiers);

            //! Add a window to the event loop.
            void addWindow(const std::weak_ptr<Window>&);

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        protected:
            void _sizeHintEvent();
            void _sizeHintEvent(
                const std::shared_ptr<IWidget>&,
                const SizeHintEvent&);

            void _drawEvent();
            void _drawEvent(
                const std::shared_ptr<IWidget>&,
                const DrawEvent&);

            std::shared_ptr<IWidget> _underCursor(
                const math::Vector2i&);
            std::shared_ptr<IWidget> _underCursor(
                const std::shared_ptr<IWidget>&,
                const math::Vector2i&);

            void _setHover(const std::shared_ptr<IWidget>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
