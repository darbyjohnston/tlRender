// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Style.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>

namespace tl
{
    namespace ui
    {
        TLRENDER_ENUM_IMPL(
            SizeRole,
            "None",
            "Margin",
            "MarginSmall",
            "MarginLarge",
            "MarginInside",
            "Spacing",
            "SpacingSmall",
            "SpacingLarge",
            "SpacingTool",
            "Border",
            "ScrollArea",
            "Handle");
        TLRENDER_ENUM_SERIALIZE_IMPL(SizeRole);

        TLRENDER_ENUM_IMPL(
            ColorRole,
            "None",
            "Window",
            "Base",
            "Button",
            "Text",
            "TextDisabled",
            "Border",
            "Hover",
            "Pressed",
            "Checked",
            "KeyFocus",
            "Overlay",
            "Red",
            "Green",
            "Blue",
            "Cyan",
            "Magenta",
            "Yellow");
        TLRENDER_ENUM_SERIALIZE_IMPL(ColorRole);

        TLRENDER_ENUM_IMPL(
            FontRole,
            "None",
            "Label",
            "Mono",
            "Title");
        TLRENDER_ENUM_SERIALIZE_IMPL(FontRole);

        struct Style::Private
        {
            std::weak_ptr<system::Context> context;
            std::map<SizeRole, int> sizeRoles;
            std::map<ColorRole, imaging::Color4f> colorRoles;
            std::map<FontRole, imaging::FontInfo> fontRoles;
        };

        void Style::_init(
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;

            p.sizeRoles[SizeRole::Margin] = 10;
            p.sizeRoles[SizeRole::MarginSmall] = 5;
            p.sizeRoles[SizeRole::MarginLarge] = 20;
            p.sizeRoles[SizeRole::MarginInside] = 2;
            p.sizeRoles[SizeRole::Spacing] = 10;
            p.sizeRoles[SizeRole::SpacingSmall] = 5;
            p.sizeRoles[SizeRole::SpacingLarge] = 20;
            p.sizeRoles[SizeRole::SpacingTool] = 2;
            p.sizeRoles[SizeRole::Border] = 1;
            p.sizeRoles[SizeRole::ScrollArea] = 200;
            p.sizeRoles[SizeRole::Handle] = 10;
            p.sizeRoles[SizeRole::HandleSmall] = 6;

            p.colorRoles[ColorRole::Window] = imaging::Color4f(.2F, .2F, .2F);
            p.colorRoles[ColorRole::Base] = imaging::Color4f(.17F, .17F, .17F);
            p.colorRoles[ColorRole::Button] = imaging::Color4f(.3F, .3F, .3F);
            p.colorRoles[ColorRole::Text] = imaging::Color4f(1.F, 1.F, 1.F);
            p.colorRoles[ColorRole::TextDisabled] = imaging::Color4f(.5F, .5F, .5F);
            p.colorRoles[ColorRole::Border] = imaging::Color4f(.13F, .13F, .13F);
            p.colorRoles[ColorRole::Hover] = imaging::Color4f(1.F, 1.F, 1.F, .1F);
            p.colorRoles[ColorRole::Pressed] = imaging::Color4f(1.F, 1.F, 1.F, .2F);
            p.colorRoles[ColorRole::Checked] = imaging::Color4f(.6F, .4F, .2F);
            p.colorRoles[ColorRole::KeyFocus] = imaging::Color4f(.6F, .6F, .4F);
            p.colorRoles[ColorRole::Overlay] = imaging::Color4f(0.F, 0.F, 0.F, .5F);
            p.colorRoles[ColorRole::Red] = imaging::Color4f(.6F, .3F, .3F);
            p.colorRoles[ColorRole::Green] = imaging::Color4f(.3F, .6F, .3F);
            p.colorRoles[ColorRole::Blue] = imaging::Color4f(.3F, .3F, .6F);
            p.colorRoles[ColorRole::Cyan] = imaging::Color4f(.3F, .6F, .6F);
            p.colorRoles[ColorRole::Magenta] = imaging::Color4f(.6F, .3F, .6F);
            p.colorRoles[ColorRole::Yellow] = imaging::Color4f(.6F, .6F, .3F);

            p.fontRoles[FontRole::Label] = imaging::FontInfo("NotoSans-Regular", 12 * 1);
            p.fontRoles[FontRole::Mono] = imaging::FontInfo("NotoMono-Regular", 12 * 1);
            p.fontRoles[FontRole::Title] = imaging::FontInfo("NotoSans-Regular", 24 * 1);
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

        int Style::getSizeRole(SizeRole value, float scale) const
        {
            TLRENDER_P();
            const auto i = p.sizeRoles.find(value);
            return (i != p.sizeRoles.end() ? i->second : 0) * scale;
        }

        imaging::Color4f Style::getColorRole(ColorRole value) const
        {
            TLRENDER_P();
            const auto i = p.colorRoles.find(value);
            return i != p.colorRoles.end() ? i->second : imaging::Color4f();
        }

        imaging::FontInfo Style::getFontRole(FontRole value, float scale) const
        {
            TLRENDER_P();
            imaging::FontInfo out;
            const auto i = p.fontRoles.find(value);
            if (i != p.fontRoles.end())
            {
                out = i->second;
            }
            out.size *= scale;
            return out;
        }
    }
}
