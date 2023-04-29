// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Event.h>

namespace tl
{
    namespace ui
    {
        imaging::FontMetrics SizeEvent::getFontMetrics(FontRole role) const
        {
            imaging::FontMetrics out;
            const auto i = fontMetrics.find(role);
            if (i != fontMetrics.end())
            {
                out = i->second;
            }
            return out;
        }

        imaging::FontMetrics DrawEvent::getFontMetrics(FontRole role) const
        {
            imaging::FontMetrics out;
            const auto i = fontMetrics.find(role);
            if (i != fontMetrics.end())
            {
                out = i->second;
            }
            return out;
        }
    }
}
