// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <string>

namespace tl
{
    namespace timeline
    {
        //! OpenColorIO options.
        struct OCIOOptions
        {
            bool        enabled  = false;
            std::string fileName;
            std::string input;
            std::string display;
            std::string view;
            std::string look;

            bool operator == (const OCIOOptions&) const;
            bool operator != (const OCIOOptions&) const;
        };
    }
}

#include <tlTimeline/OCIOOptionsInline.h>
