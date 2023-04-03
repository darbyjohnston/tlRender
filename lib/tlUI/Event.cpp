// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Event.h>

namespace tl
{
    namespace ui
    {
        imaging::FontInfo SizeEvent::getFontInfo(FontRole role) const
        {
            imaging::FontInfo out;
            const auto i = fontInfo.find(role);
            if (i != fontInfo.end())
            {
                out = i->second;
            }
            return out;
        }

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

        imaging::FontInfo DrawEvent::getFontInfo(FontRole role) const
        {
            imaging::FontInfo out;
            const auto i = fontInfo.find(role);
            if (i != fontInfo.end())
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
