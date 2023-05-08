// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IconLibrary.h>
#include <tlUI/Style.h>

#include <tlTimeline/IRender.h>

#include <tlCore/FontSystem.h>

namespace tl
{
    namespace ui
    {
        class IWidget;

        //! Child event.
        struct ChildEvent
        {
            std::shared_ptr<IWidget> child;
        };

        //! Tick event.
        struct TickEvent
        {
            std::shared_ptr<Style> style;
            std::shared_ptr<IconLibrary> iconLibrary;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            float displayScale = 1.F;
        };

        //! Size hint event.
        struct SizeHintEvent
        {
            std::shared_ptr<Style> style;
            std::shared_ptr<IconLibrary> iconLibrary;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            float displayScale = 1.F;
            std::map<FontRole, imaging::FontMetrics> fontMetrics;

            imaging::FontMetrics getFontMetrics(FontRole) const;
        };

        //! Clip event.
        struct ClipEvent
        {
            std::shared_ptr<Style> style;
            std::shared_ptr<IconLibrary> iconLibrary;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            float displayScale = 1.F;
            std::map<FontRole, imaging::FontMetrics> fontMetrics;

            imaging::FontMetrics getFontMetrics(FontRole) const;
        };

        //! Draw event.
        struct DrawEvent
        {
            std::shared_ptr<Style> style;
            std::shared_ptr<IconLibrary> iconLibrary;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            float displayScale = 1.F;
            std::map<FontRole, imaging::FontMetrics> fontMetrics;
            std::shared_ptr<IWidget> focusWidget;

            imaging::FontMetrics getFontMetrics(FontRole) const;
        };

        //! Mouse move event.
        struct MouseMoveEvent
        {
            math::Vector2i pos;
            math::Vector2i prev;
            bool accept = false;
        };

        //! Scroll event (mouse wheel or touch pad).
        struct ScrollEvent
        {
            math::Vector2i pos;
            float dx = 0.F;
            float dy = 0.F;
            bool accept = false;
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
            math::Vector2i pos;
            bool accept = false;
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
            RightSuper,

            Count,
            First = Unknown
        };
        TLRENDER_ENUM(Key);
        TLRENDER_ENUM_SERIALIZE(Key);

        //! Key event.
        struct KeyEvent
        {
            Key key = Key::Unknown;
            int modifiers = 0;
            math::Vector2i pos;
            bool accept = false;
        };

        //! Text event.
        struct TextEvent
        {
            std::string text;
            bool accept = false;
        };
    }
}
