// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <string>

namespace tl
{
    namespace core
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
        }
    }
}

#include <tlCore/OCIOInline.h>
