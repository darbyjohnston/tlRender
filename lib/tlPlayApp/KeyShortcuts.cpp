// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/KeyShortcuts.h>

namespace tl
{
    namespace play
    {
        KeyShortcut::KeyShortcut(
            const std::string& name,
            dtk::Key key,
            int modifiers) :
            name(name),
            key(key),
            modifiers(modifiers)
        {}

        bool KeyShortcut::operator == (const KeyShortcut& other) const
        {
            return
                name == other.name &&
                key == other.key &&
                modifiers == other.modifiers;
        }

        bool KeyShortcut::operator != (const KeyShortcut& other) const
        {
            return !(*this == other);
        }

        void to_json(nlohmann::json& json, const KeyShortcut& in)
        {
            json["Name"] = in.name;
            json["Key"] = to_string(in.key);
            json["Modifiers"] = in.modifiers;
        }

        void from_json(const nlohmann::json& json, KeyShortcut& out)
        {
            json.at("Name").get_to(out.name);
            from_string(json.at("Key").get<std::string>(), out.key);
            json.at("Modifiers").get_to(out.modifiers);
        }
    }
}
