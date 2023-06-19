// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace ui
    {
        inline int Style::getSizeRole(SizeRole role, float scale) const
        {
            return _sizeRoles[static_cast<size_t>(role)] * scale;
        }

        inline imaging::Color4f Style::getColorRole(ColorRole role) const
        {
            return _colorRoles[static_cast<size_t>(role)];
        }

        inline imaging::FontInfo Style::getFontRole(FontRole role, float scale) const
        {
            imaging::FontInfo out = _fontRoles[static_cast<size_t>(role)];
            out.size *= scale;
            return out;
        }
    }
}
