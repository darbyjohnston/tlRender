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
            explicit KeyShortcut(dtk::Key, int modifiers = 0);

            dtk::Key key       = dtk::Key::Unknown;
            int      modifiers = 0;

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
