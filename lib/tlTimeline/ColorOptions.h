// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/Util.h>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace tl
{
    namespace timeline
    {
        //! OpenColorIO configuration options.
        enum class OCIOConfig
        {
            BuiltIn,
            EnvVar,
            File,

            Count,
            First = BuiltIn
        };
        FTK_ENUM(OCIOConfig);

        //! OpenColorIO options.
        struct OCIOOptions
        {
            bool        enabled  = false;
            OCIOConfig  config   = OCIOConfig::BuiltIn;
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
        FTK_ENUM(LUTOrder);

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

        void to_json(nlohmann::json&, const OCIOOptions&);
        void to_json(nlohmann::json&, const LUTOptions&);

        void from_json(const nlohmann::json&, OCIOOptions&);
        void from_json(const nlohmann::json&, LUTOptions&);

        ///@}
    }
}
