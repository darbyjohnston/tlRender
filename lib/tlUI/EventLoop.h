// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Window.h>

namespace tl
{
    //! User interface.
    namespace ui
    {
        //! Event loop.
        class EventLoop : public std::enable_shared_from_this<EventLoop>
        {
            TLRENDER_NON_COPYABLE(EventLoop);

        protected:
            void _init(
                const std::shared_ptr<Style>&,
                const std::shared_ptr<IconLibrary>&,
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<system::Context>&);

            EventLoop();

        public:
            ~EventLoop();

            //! Create a new event loop.
            static std::shared_ptr<EventLoop> create(
                const std::shared_ptr<Style>&,
                const std::shared_ptr<IconLibrary>&,
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<imaging::FontSystem>&,
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

            //! Tick the event loop.
            void tick();

        protected:
            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<IWidget>&,
                const TickEvent&);

            void _sizeEvent();
            void _sizeEvent(
                const std::shared_ptr<IWidget>&,
                const SizeEvent&);

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
