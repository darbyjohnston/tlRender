// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/LayoutUtil.h>

#include <dtk/core/Math.h>

#include <sstream>

namespace tl
{
    namespace ui
    {
        dtk::Box2I align(
            const dtk::Box2I&    box,
            const dtk::Size2I& sizeHint,
            Stretch               hStretch,
            Stretch               vStretch,
            HAlign                hAlign,
            VAlign                vAlign)
        {
            dtk::V2I pos;
            dtk::V2I size;

            switch (hStretch)
            {
            case Stretch::Fixed:
                switch (hAlign)
                {
                case HAlign::Left:
                    pos.x = box.x();
                    break;
                case HAlign::Center:
                    pos.x = box.x() + box.w() / 2 - sizeHint.w / 2;
                    break;
                case HAlign::Right:
                    pos.x = box.x() + box.w() - sizeHint.w;
                    break;
                }
                size.x = sizeHint.w;
                break;
            case Stretch::Expanding:
                pos.x = box.x();
                size.x = box.w();
                break;
            }

            switch (vStretch)
            {
            case Stretch::Fixed:
                switch (vAlign)
                {
                case VAlign::Top:
                    pos.y = box.y();
                    break;
                case VAlign::Center:
                    pos.y = box.y() + box.h() / 2 - sizeHint.h / 2;
                    break;
                case VAlign::Bottom:
                    pos.y = box.y() + box.w() - sizeHint.h;
                    break;
                }
                size.y = sizeHint.h;
                break;
            case Stretch::Expanding:
                pos.y = box.y();
                size.y = box.h();
                break;
            }

            return dtk::Box2I(pos.x, pos.y, size.x, size.y);
        }

        std::string format(int value)
        {
            std::stringstream ss;
            ss << std::setfill('0');
            ss << std::setw(dtk::digits(value));
            ss << 0;
            return ss.str();
        }

        std::string format(float value, int precision)
        {
            std::stringstream ss;
            ss.precision(precision);
            ss << std::fixed;
            ss << std::setfill('0');
            ss << std::setw(dtk::digits(value));
            ss << 0;
            return ss.str();
        }
    }
}
