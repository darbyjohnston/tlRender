// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Event.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace ui
    {
        imaging::FontMetrics SizeHintEvent::getFontMetrics(FontRole role) const
        {
            imaging::FontMetrics out;
            const auto i = fontMetrics.find(role);
            if (i != fontMetrics.end())
            {
                out = i->second;
            }
            return out;
        }

        imaging::FontMetrics DrawEvent::getFontMetrics(FontRole role) const
        {
            imaging::FontMetrics out;
            const auto i = fontMetrics.find(role);
            if (i != fontMetrics.end())
            {
                out = i->second;
            }
            return out;
        }

        TLRENDER_ENUM_IMPL(
            Key,
            "Unknown",
            "Space",
            "Apostrophe",
            "Comma",
            "Minus",
            "Period",
            "Slash",
            "_0",
            "_1",
            "_2",
            "_3",
            "_4",
            "_5",
            "_6",
            "_7",
            "_8",
            "_9",
            "Semicolon",
            "Equal",
            "A",
            "B",
            "C",
            "D",
            "E",
            "F",
            "G",
            "H",
            "I",
            "J",
            "K",
            "L",
            "M",
            "N",
            "O",
            "P",
            "Q",
            "R",
            "S",
            "T",
            "U",
            "V",
            "W",
            "X",
            "Y",
            "Z",
            "LeftBracket",
            "Backslash",
            "RightBracket",
            "GraveAccent",
            "Escape",
            "Enter",
            "Tab",
            "Backspace",
            "Insert",
            "Delete",
            "Right",
            "Left",
            "Down",
            "Up",
            "PageUp",
            "PageDown",
            "Home",
            "End",
            "CapsLock",
            "ScrollLock",
            "NumLock",
            "PrintScreen",
            "Pause",
            "F1",
            "F2",
            "F3",
            "F4",
            "F5",
            "F6",
            "F7",
            "F8",
            "F9",
            "F10",
            "F11",
            "F12",
            "LeftShift",
            "LeftControl",
            "LeftAlt",
            "LeftSuper",
            "RightShift",
            "RightControl",
            "RightAlt",
            "RightSuper");
        TLRENDER_ENUM_SERIALIZE_IMPL(Key);
    }
}
