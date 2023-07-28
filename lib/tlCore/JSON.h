// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

namespace tl
{
    void to_json(nlohmann::json&, bool);
    void to_json(nlohmann::json&, int);
    void to_json(nlohmann::json&, float);
    void to_json(nlohmann::json&, double);
    void to_json(nlohmann::json&, const std::string&);

    void from_json(const nlohmann::json&, bool&);
    void from_json(const nlohmann::json&, int&);
    void from_json(const nlohmann::json&, float&);
    void from_json(const nlohmann::json&, double&);
    void from_json(const nlohmann::json&, std::string&);
}
