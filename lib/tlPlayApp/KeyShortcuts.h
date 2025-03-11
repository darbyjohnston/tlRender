// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Event.h>

namespace tl
{
    namespace play
    {
        //! Keyboard shortcut.
        struct KeyShortcut
        {
            KeyShortcut() = default;
            KeyShortcut(
                const std::string& name,
                const std::string& text,
                dtk::Key = dtk::Key::Unknown,
                int modifiers = 0);

            std::string name;
            std::string text;
            dtk::Key    key       = dtk::Key::Unknown;
            int         modifiers = 0;

            bool operator == (const KeyShortcut&) const;
            bool operator != (const KeyShortcut&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const KeyShortcut&);

        void from_json(const nlohmann::json&, KeyShortcut&);

        ///@}
    }
}
