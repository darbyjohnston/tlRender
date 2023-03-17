// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Style.h>

#include <tlTimeline/IRender.h>

#include <tlCore/FontSystem.h>

namespace tl
{
    namespace ui
    {
        //! Size hint event.
        struct SizeHintEvent
        {
            std::shared_ptr<Style> style;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            float contentScale = 1.F;
        };

        //! Draw event.
        struct DrawEvent
        {
            std::shared_ptr<Style> style;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            std::shared_ptr<timeline::IRender> render;
            float contentScale = 1.F;
        };

        //! Mouse move event.
        struct MouseMoveEvent
        {
            math::Vector2i pos;
            math::Vector2i prev;
        };

        //! Keyboard modifiers.
        enum class KeyModifier
        {
            None    = 0,
            Shift   = 1,
            Control = 2,
            Alt     = 4
        };

        //! Mouse click event.
        struct MouseClickEvent
        {
            int button = 0;
            int modifiers = 0;
        };

        //! Keys.
        enum class Key
        {
            Unknown,
            Space,
            Apostrophe, // '
            Comma,      // ,
            Minus,      // -
            Period,     // .
            Slash,      // /
            _0,
            _1,
            _2,
            _3,
            _4,
            _5,
            _6,
            _7,
            _8,
            _9,
            Semicolon, // ;
            Equal,     // = 
            A,
            B,
            C,
            D,
            E,
            F,
            G,
            H,
            I,
            J,
            K,
            L,
            M,
            N,
            O,
            P,
            Q,
            R,
            S,
            T,
            U,
            V,
            W,
            X,
            Y,
            Z,
            LeftBracket,  // [
            Backslash,    // '\'
            RightBracket, // ]
            GraveAccent,  // `
            Escape,
            Enter,
            Tab,
            Backspace,
            Insert,
            Delete,
            Right,
            Left,
            Down,
            Up,
            PageUp,
            PageDown,
            Home,
            End,
            CapsLock,
            ScrollLock,
            NumLock,
            PrintScreen,
            Pause,
            F1,
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12,
            LeftShift,
            LeftControl,
            LeftAlt,
            LeftSuper,
            RightShift,
            RightControl,
            RightAlt,
            RightSuper
        };

        //! Key event.
        struct KeyEvent
        {
            Key key = Key::Unknown;
        };
    }
}
