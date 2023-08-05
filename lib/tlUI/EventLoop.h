// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>

#include <list>

namespace tl
{
    //! User interface.
    namespace ui
    {
        class IClipboard;

        //! Cursors.
        enum class StandardCursor
        {
            Arrow,
            IBeam,
            Crosshair,
            Hand,
            HResize,
            VResize
        };

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

            //! Add a top level widget.
            void addWidget(const std::shared_ptr<IWidget>&);

            //! Remove a top level widget.
            void removeWidget(const std::shared_ptr<IWidget>&);

            //! Set the user interface display resolution.
            void setDisplaySize(const image::Size&);

            //! Set the user interface display scale. This will scale the size
            //! roles, fonts, and other metrics to support different
            //! resolutions.
            void setDisplayScale(float);

            //! Set the key focus widget.
            void setKeyFocus(const std::shared_ptr<IWidget>&);

            //! Handle key presses.
            bool key(Key, bool press, int modifiers);

            //! Handle text input.
            void text(const std::string&);

            //! Handle the cursor entering and leaving.
            void cursorEnter(bool enter);

            //! Handle the cursor position.
            void cursorPos(const math::Vector2i&);

            //! Handle mouse button presses.
            void mouseButton(int button, bool press, int modifiers);

            //! Set the standard cursor function.
            void setCursor(const std::function<void(StandardCursor)>&);

            //! Set the custom cursor function.
            void setCursor(const std::function<void(
                const std::shared_ptr<image::Image>&,
                const math::Vector2i&)>&);

            //! Handle scrolling (mouse wheel or touch pad).
            void scroll(float dx, float dy, int modifiers);

            //! Get the clipboard.
            const std::shared_ptr<IClipboard>& getClipboard() const;

            //! Tick the event loop.
            void tick();

            //! Get whether a draw update is needed.
            bool hasDrawUpdate() const;

            //! Draw the user interface.
            void draw(const std::shared_ptr<timeline::IRender>&);

            //! Take a screenshot of a widget.
            std::shared_ptr<image::Image> screenshot(
                const std::shared_ptr<IWidget>&);

            //! Set the screenshot capture function.
            void setCapture(const std::function<std::shared_ptr<image::Image>(
                const math::BBox2i&)>&);

        protected:
            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<IWidget>&,
                bool visible,
                bool enabled,
                const TickEvent&);

            bool _getSizeUpdate();
            bool _getSizeUpdate(const std::shared_ptr<IWidget>&);
            void _sizeHintEvent();
            void _sizeHintEvent(
                const std::shared_ptr<IWidget>&,
                const SizeHintEvent&);

            void _clipEvent();
            void _clipEvent(
                const std::shared_ptr<IWidget>&,
                const math::BBox2i&,
                bool clipped,
                const ClipEvent&);

            bool _getDrawUpdate();
            bool _getDrawUpdate(const std::shared_ptr<IWidget>&);
            void _drawEvent(const std::shared_ptr<timeline::IRender>&);
            void _drawEvent(
                const std::shared_ptr<IWidget>&,
                const math::BBox2i&,
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

            void _purgeTopLevelWidgets();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
