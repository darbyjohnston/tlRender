// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IconLibrary.h>
#include <tlUI/Style.h>

#include <tlTimeline/IRender.h>

#include <dtk/core/FontSystem.h>

namespace tl
{
    namespace ui
    {
        class IWidget;

        //! Child event.
        struct ChildEvent
        {
            ChildEvent();
            ChildEvent(const std::shared_ptr<IWidget>&);

            std::shared_ptr<IWidget> child;
        };

        //! Tick event.
        struct TickEvent
        {
            TickEvent();
            TickEvent(
                const std::shared_ptr<Style>&           style,
                const std::shared_ptr<IconLibrary>&     iconLibrary,
                const std::shared_ptr<dtk::FontSystem>& fontSystem);

            std::shared_ptr<Style>           style;
            std::shared_ptr<IconLibrary>     iconLibrary;
            std::shared_ptr<dtk::FontSystem> fontSystem;
        };

        //! Size hint event.
        struct SizeHintEvent
        {
            SizeHintEvent();
            SizeHintEvent(
                const std::shared_ptr<Style>&           style,
                const std::shared_ptr<IconLibrary>&     iconLibrary,
                const std::shared_ptr<dtk::FontSystem>& fontSystem,
                float                                   displayScale);

            std::shared_ptr<Style>           style;
            std::shared_ptr<IconLibrary>     iconLibrary;
            std::shared_ptr<dtk::FontSystem> fontSystem;
            float                            displayScale = 1.F;
        };

        //! Draw event.
        struct DrawEvent
        {
            DrawEvent();
            DrawEvent(
                const std::shared_ptr<Style>&             style,
                const std::shared_ptr<IconLibrary>&       iconLibrary,
                const std::shared_ptr<timeline::IRender>& render,
                const std::shared_ptr<dtk::FontSystem>&   fontSystem);

            std::shared_ptr<Style>             style;
            std::shared_ptr<IconLibrary>       iconLibrary;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<dtk::FontSystem>   fontSystem;
        };

        //! Drag and drop data.
        class DragAndDropData : public std::enable_shared_from_this<DragAndDropData>
        {
        public:
            virtual ~DragAndDropData() = 0;
        };

        //! Mouse move event.
        struct MouseMoveEvent
        {
            MouseMoveEvent();
            MouseMoveEvent(
                const dtk::V2I& pos,
                const dtk::V2I& prev);

            dtk::V2I                   pos;
            dtk::V2I                   prev;
            bool                             accept = false;
            std::shared_ptr<DragAndDropData> dndData;
            std::shared_ptr<dtk::Image>    dndCursor;
            dtk::V2I                   dndCursorHotspot;
        };

        //! Keyboard modifiers.
        enum class KeyModifier
        {
            None    = 0,
            Shift   = 1,
            Control = 2,
            Alt     = 4,
            Super   = 8
        };

        //! OS specific command key modifier.
#if defined(__APPLE__)
        const KeyModifier commandKeyModifier = KeyModifier::Super;
#else // __APPLE__
        const KeyModifier commandKeyModifier = KeyModifier::Control;
#endif // __APPLE__

        //! Get a keyboard modifier label.
        std::string getKeyModifierLabel(int);

        //! Mouse click event.
        struct MouseClickEvent
        {
            MouseClickEvent();
            MouseClickEvent(
                int                   button,
                int                   modifiers,
                const dtk::V2I& pos);

            int            button    = 0;
            int            modifiers = 0;
            dtk::V2I pos;
            bool           accept    = false;
        };

        //! Scroll event (mouse wheel or touch pad).
        struct ScrollEvent
        {
            ScrollEvent();
            ScrollEvent(
                const dtk::V2F& value,
                int                   modifiers,
                const dtk::V2I& pos);

            dtk::V2F value;
            int            modifiers = 0;
            dtk::V2I pos;
            bool           accept    = false;
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

        //! Get a keyboard shortcut label.
        std::string getLabel(Key, int modifiers = 0);

        //! Key event.
        struct KeyEvent
        {
            KeyEvent();
            KeyEvent(
                Key                   key,
                int                   modifiers,
                const dtk::V2I& pos);

            Key            key       = Key::Unknown;
            int            modifiers = 0;
            dtk::V2I pos;
            bool           accept    = false;
        };

        //! Text event.
        struct TextEvent
        {
            TextEvent();
            TextEvent(const std::string& text);

            std::string text;
            bool        accept = false;
        };

        //! Drag and drop text data.
        class TextDragAndDropData : public DragAndDropData
        {
        public:
            TextDragAndDropData(const std::string& text);

            virtual ~TextDragAndDropData();

            const std::string& getText() const;

        private:
            std::string _text;
        };

        //! Drag and drop event.
        struct DragAndDropEvent
        {
            DragAndDropEvent();
            DragAndDropEvent(
                const dtk::V2I&                   pos,
                const dtk::V2I&                   prev,
                const std::shared_ptr<DragAndDropData>& data);

            dtk::V2I                   pos;
            dtk::V2I                   prev;
            std::shared_ptr<DragAndDropData> data;
            bool                             accept = false;
        };
    }
}
