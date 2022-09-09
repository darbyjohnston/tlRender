// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace tl
{
    namespace timeline
    {
        //! Color configuration options.
        struct ColorConfigOptions
        {
            std::string fileName;
            std::string input;
            std::string display;
            std::string view;
            std::string look;

            bool operator == (const ColorConfigOptions&) const;
            bool operator != (const ColorConfigOptions&) const;
        };

        void to_json(nlohmann::json&, const ColorConfigOptions&);

        void from_json(const nlohmann::json&, ColorConfigOptions&);
    }
}

#include <tlTimeline/ColorConfigOptionsInline.h>
