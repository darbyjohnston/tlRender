// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Util.h>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace tl
{
    namespace timeline
    {
        //! OpenColorIO options.
        struct OCIOOptions
        {
            bool        enabled = false;
            std::string fileName;
            std::string input;
            std::string display;
            std::string view;
            std::string look;

            bool operator == (const OCIOOptions&) const;
            bool operator != (const OCIOOptions&) const;
        };

        //! LUT operation order.
        enum class LUTOrder
        {
            PostColorConfig,
            PreColorConfig,

            Count,
            First = PostColorConfig
        };
        DTK_ENUM(LUTOrder);

        //! LUT options.
        struct LUTOptions
        {
            bool        enabled  = false;
            std::string fileName;
            LUTOrder    order    = LUTOrder::First;

            bool operator == (const LUTOptions&) const;
            bool operator != (const LUTOptions&) const;
        };

        //! Get the list of LUT format names.
        std::vector<std::string> getLUTFormatNames();

        //! Get the list of LUT format file extensions.
        std::vector<std::string> getLUTFormatExtensions();

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const LUTOptions&);
        void to_json(nlohmann::json&, const OCIOOptions&);

        void from_json(const nlohmann::json&, LUTOptions&);
        void from_json(const nlohmann::json&, OCIOOptions&);

        ///@}
    }
}

#include <tlTimeline/ColorOptionsInline.h>
