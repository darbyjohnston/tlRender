// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace tl
{
    namespace imaging
    {
        //! OpenColorIO configuration.
        struct ColorConfig
        {
            std::string fileName;
            std::string input;
            std::string display;
            std::string view;
            std::string look;

            bool operator == (const ColorConfig&) const;
            bool operator != (const ColorConfig&) const;
        };

        void to_json(nlohmann::json&, const ColorConfig&);

        void from_json(const nlohmann::json&, ColorConfig&);
    }
}

#include <tlCore/ColorConfigInline.h>
