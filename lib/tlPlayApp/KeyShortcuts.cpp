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
            const std::string& text,
            dtk::Key key,
            int modifiers) :
            name(name),
            text(text),
            key(key),
            modifiers(modifiers)
        {}

        bool KeyShortcut::operator == (const KeyShortcut& other) const
        {
            return
                name == other.name &&
                text == other.text &&
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
            json["Text"] = in.text;
            json["Key"] = to_string(in.key);
            json["Modifiers"] = in.modifiers;
        }

        void from_json(const nlohmann::json& json, KeyShortcut& out)
        {
            json.at("Name").get_to(out.name);
            json.at("Text").get_to(out.text);
            from_string(json.at("Key").get<std::string>(), out.key);
            json.at("Modifiers").get_to(out.modifiers);
        }
    }
}
