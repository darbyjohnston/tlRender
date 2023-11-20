// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>

#include <tlCore/ListObserver.h>

#include <list>

namespace tl
{
    //! User interface.
    namespace ui
    {
        class IClipboard;
        class Window;

        //! Event loop.
        class EventLoop : public std::enable_shared_from_this<EventLoop>
        {
            TLRENDER_NON_COPYABLE(EventLoop);

        protected:
            void _init(
                const std::shared_ptr<Style>&,
                const std::shared_ptr<IconLibrary>&,
                const std::shared_ptr<IClipboard>&,
                const std::shared_ptr<system::Context>&);

            EventLoop();

        public:
            ~EventLoop();

            //! Create a new event loop.
            static std::shared_ptr<EventLoop> create(
                const std::shared_ptr<Style>&,
                const std::shared_ptr<IconLibrary>&,
                const std::shared_ptr<IClipboard>&,
                const std::shared_ptr<system::Context>&);

            //! Add a window.
            void addWindow(const std::shared_ptr<Window>&);

            //! Remove a window.
            void removeWindow(const std::shared_ptr<Window>&);

            //! Observe the windows.
            std::shared_ptr<observer::IList<std::shared_ptr<Window> > > observeWindows() const;

            //! Set the window resolution.
            void setWindowResolution(
                const std::shared_ptr<Window>&,
                const math::Size2i&);

            //! Set the window scale.
            void setWindowScale(
                const std::shared_ptr<Window>&,
                float);

            //! Set the key focus widget.
            void setKeyFocus(const std::shared_ptr<IWidget>&);

            //! Handle key presses.
            bool key(
                const std::shared_ptr<Window>&,
                Key,
                bool press,
                int modifiers);

            //! Handle text input.
            void text(
                const std::shared_ptr<Window>&,
                const std::string&);

            //! Handle the cursor entering and leaving.
            void cursorEnter(
                const std::shared_ptr<Window>&,
                bool enter);

            //! Handle the cursor position.
            void cursorPos(
                const std::shared_ptr<Window>&,
                const math::Vector2i&);

            //! Handle mouse button presses.
            void mouseButton(
                const std::shared_ptr<Window>&,
                int button,
                bool press,
                int modifiers);

            //! Handle scrolling (mouse wheel or touch pad).
            void scroll(
                const std::shared_ptr<Window>&,
                const math::Vector2f&,
                int modifiers);

            //! Get the clipboard.
            const std::shared_ptr<IClipboard>& getClipboard() const;

            //! Tick the event loop.
            void tick();

            //! Get whether a draw update is needed.
            bool hasDrawUpdate(const std::shared_ptr<Window>&) const;

            //! Draw the window.
            void draw(
                const std::shared_ptr<Window>&,
                const std::shared_ptr<timeline::IRender>&);

        protected:
            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<IWidget>&,
                bool visible,
                bool enabled,
                const TickEvent&);

            bool _getSizeUpdate(const std::shared_ptr<IWidget>&);
            void _sizeHintEvent(const std::shared_ptr<Window>&);
            void _sizeHintEvent(
                const std::shared_ptr<IWidget>&,
                const SizeHintEvent&);

            void _clipEvent(const std::shared_ptr<Window>&);
            void _clipEvent(
                const std::shared_ptr<IWidget>&,
                const math::Box2i&,
                bool clipped);

            bool _getDrawUpdate(const std::shared_ptr<IWidget>&);
            void _drawEvent(
                const std::shared_ptr<Window>&,
                const std::shared_ptr<timeline::IRender>&);
            void _drawEvent(
                const std::shared_ptr<IWidget>&,
                const math::Box2i&,
                const DrawEvent&);

            std::list<std::shared_ptr<IWidget> > _getUnderCursor(const math::Vector2i&);
            void _getUnderCursor(
                const std::shared_ptr<IWidget>&,
                const math::Vector2i&,
                std::list<std::shared_ptr<IWidget> >&);

            void _setHover(const std::shared_ptr<IWidget>&);
            void _hoverUpdate(MouseMoveEvent&);

            std::shared_ptr<IWidget> _keyFocusNext(const std::shared_ptr<IWidget>&);
            std::shared_ptr<IWidget> _keyFocusPrev(const std::shared_ptr<IWidget>&);
            void _getKeyFocus(
                const std::shared_ptr<IWidget>&,
                std::list<std::shared_ptr<IWidget> >&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
