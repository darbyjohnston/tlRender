// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Style.h>

namespace tl
{
    namespace ui
    {
        imaging::Color4f lighter(const imaging::Color4f& color, float amount)
        {
            return imaging::Color4f(
                color.r + amount,
                color.g + amount,
                color.b + amount,
                color.a);
        }

        imaging::Color4f darker(const imaging::Color4f& color, float amount)
        {
            return imaging::Color4f(
                color.r - amount,
                color.g - amount,
                color.b - amount,
                color.a);
        }

        struct Style::Private
        {
            std::weak_ptr<system::Context> context;
            std::map<SizeRole, int> sizeRoles;
            std::map<ColorRole, imaging::Color4f> colorRoles;
        };

        void Style::_init(
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;

            p.sizeRoles[SizeRole::Margin] = 10;
            p.sizeRoles[SizeRole::Spacing] = 10;
            p.sizeRoles[SizeRole::Border] = 1;

            p.colorRoles[ColorRole::Window] = imaging::Color4f(.2F, .2F, .2F);
            p.colorRoles[ColorRole::Button] = imaging::Color4f(.3F, .3F, .3F);
            p.colorRoles[ColorRole::Text] = imaging::Color4f(1.F, 1.F, 1.F);
            p.colorRoles[ColorRole::Hover] = imaging::Color4f(1.F, 1.F, 1.F, .1F);
            p.colorRoles[ColorRole::Pressed] = imaging::Color4f(1.F, 1.F, 1.F, .3F);
        }

        Style::Style() :
            _p(new Private)
        {}

        Style::~Style()
        {}

        std::shared_ptr<Style> Style::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<Style>(new Style);
            out->_init(context);
            return out;
        }

        int Style::getSizeRole(SizeRole value) const
        {
            TLRENDER_P();
            const auto i = p.sizeRoles.find(value);
            return i != p.sizeRoles.end() ? i->second : 0;
        }

        imaging::Color4f Style::getColorRole(ColorRole value) const
        {
            TLRENDER_P();
            const auto i = p.colorRoles.find(value);
            return i != p.colorRoles.end() ? i->second : imaging::Color4f();
        }
    }
}
