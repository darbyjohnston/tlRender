// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/LayoutUtil.h>

#include <tlCore/Math.h>

#include <sstream>

namespace tl
{
    namespace ui
    {
        math::BBox2i align(
            const math::BBox2i&   bbox,
            const math::Vector2i& sizeHint,
            Stretch               hStretch,
            Stretch               vStretch,
            HAlign                hAlign,
            VAlign                vAlign)
        {
            math::Vector2i pos;
            math::Vector2i size;

            switch (hStretch)
            {
            case Stretch::Fixed:
                switch (hAlign)
                {
                case HAlign::Left:
                    pos.x = bbox.x();
                    break;
                case HAlign::Center:
                    pos.x = bbox.x() + bbox.w() / 2 - sizeHint.x / 2;
                    break;
                case HAlign::Right:
                    pos.x = bbox.x() + bbox.w() - sizeHint.x;
                    break;
                }
                size.x = sizeHint.x;
                break;
            case Stretch::Expanding:
                pos.x = bbox.x();
                size.x = bbox.w();
                break;
            }

            switch (vStretch)
            {
            case Stretch::Fixed:
                switch (vAlign)
                {
                case VAlign::Top:
                    pos.y = bbox.y();
                    break;
                case VAlign::Center:
                    pos.y = bbox.y() + bbox.h() / 2 - sizeHint.y / 2;
                    break;
                case VAlign::Bottom:
                    pos.y = bbox.y() + bbox.w() - sizeHint.y;
                    break;
                }
                size.y = sizeHint.y;
                break;
            case Stretch::Expanding:
                pos.y = bbox.y();
                size.y = bbox.h();
                break;
            }

            return math::BBox2i(pos.x, pos.y, size.x, size.y);
        }

        std::string format(int value)
        {
            std::stringstream ss;
            ss << std::setfill('0');
            ss << std::setw(math::digits(value));
            ss << 0;
            return ss.str();
        }

        std::string format(float value, int precision)
        {
            std::stringstream ss;
            ss.precision(precision);
            ss << std::fixed;
            ss << std::setfill('0');
            ss << std::setw(math::digits(value));
            ss << 0;
            return ss.str();
        }
    }
}
