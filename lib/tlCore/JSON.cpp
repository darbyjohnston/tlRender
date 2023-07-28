// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/JSON.h>

namespace tl
{
    void to_json(nlohmann::json& json, bool value)
    {
        json = value;
    }

    void to_json(nlohmann::json& json, int value)
    {
        json = value;
    }

    void to_json(nlohmann::json& json, float value)
    {
        json = value;
    }

    void to_json(nlohmann::json& json, double value)
    {
        json = value;
    }

    void to_json(nlohmann::json& json, const std::string& value)
    {
        json = value;
    }

    void from_json(const nlohmann::json& json, bool& value)
    {
        json.get_to(value);
    }

    void from_json(const nlohmann::json& json, int& value)
    {
        json.get_to(value);
    }

    void from_json(const nlohmann::json& json, float& value)
    {
        json.get_to(value);
    }

    void from_json(const nlohmann::json& json, double& value)
    {
        json.get_to(value);
    }

    void from_json(const nlohmann::json& json, std::string& value)
    {
        json.get_to(value);
    }
}