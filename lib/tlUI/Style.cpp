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
            "MarginDialog",
            "Spacing",
            "SpacingSmall",
            "SpacingLarge",
            "SpacingTool",
            "Border",
            "ScrollArea",
            "Slider",
            "Handle",
            "Shadow",
            "DragLength");
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
            std::shared_ptr<observer::Value<bool> > changed;
        };

        void Style::_init(
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;

            _sizeRoles.resize(static_cast<size_t>(SizeRole::Count));
            _sizeRoles[static_cast<size_t>(SizeRole::Margin)] = 10;
            _sizeRoles[static_cast<size_t>(SizeRole::MarginSmall)] = 5;
            _sizeRoles[static_cast<size_t>(SizeRole::MarginLarge)] = 20;
            _sizeRoles[static_cast<size_t>(SizeRole::MarginInside)] = 2;
            _sizeRoles[static_cast<size_t>(SizeRole::MarginDialog)] = 40;
            _sizeRoles[static_cast<size_t>(SizeRole::Spacing)] = 10;
            _sizeRoles[static_cast<size_t>(SizeRole::SpacingSmall)] = 5;
            _sizeRoles[static_cast<size_t>(SizeRole::SpacingLarge)] = 20;
            _sizeRoles[static_cast<size_t>(SizeRole::SpacingTool)] = 2;
            _sizeRoles[static_cast<size_t>(SizeRole::Border)] = 1;
            _sizeRoles[static_cast<size_t>(SizeRole::ScrollArea)] = 200;
            _sizeRoles[static_cast<size_t>(SizeRole::Slider)] = 100;
            _sizeRoles[static_cast<size_t>(SizeRole::Handle)] = 10;
            _sizeRoles[static_cast<size_t>(SizeRole::HandleSmall)] = 6;
            _sizeRoles[static_cast<size_t>(SizeRole::Shadow)] = 15;
            _sizeRoles[static_cast<size_t>(SizeRole::DragLength)] = 10;

            _colorRoles.resize(static_cast<size_t>(ColorRole::Count));
            _colorRoles[static_cast<size_t>(ColorRole::Window)] = imaging::Color4f(.2F, .2F, .2F);
            _colorRoles[static_cast<size_t>(ColorRole::Base)] = imaging::Color4f(.17F, .17F, .17F);
            _colorRoles[static_cast<size_t>(ColorRole::Button)] = imaging::Color4f(.3F, .3F, .3F);
            _colorRoles[static_cast<size_t>(ColorRole::Text)] = imaging::Color4f(1.F, 1.F, 1.F);
            _colorRoles[static_cast<size_t>(ColorRole::TextDisabled)] = imaging::Color4f(.5F, .5F, .5F);
            _colorRoles[static_cast<size_t>(ColorRole::Border)] = imaging::Color4f(.13F, .13F, .13F);
            _colorRoles[static_cast<size_t>(ColorRole::Hover)] = imaging::Color4f(1.F, 1.F, 1.F, .1F);
            _colorRoles[static_cast<size_t>(ColorRole::Pressed)] = imaging::Color4f(1.F, 1.F, 1.F, .2F);
            _colorRoles[static_cast<size_t>(ColorRole::Checked)] = imaging::Color4f(.6F, .4F, .2F);
            _colorRoles[static_cast<size_t>(ColorRole::KeyFocus)] = imaging::Color4f(.6F, .6F, .4F);
            _colorRoles[static_cast<size_t>(ColorRole::Overlay)] = imaging::Color4f(0.F, 0.F, 0.F, .5F);
            _colorRoles[static_cast<size_t>(ColorRole::Red)] = imaging::Color4f(.6F, .3F, .3F);
            _colorRoles[static_cast<size_t>(ColorRole::Green)] = imaging::Color4f(.3F, .6F, .3F);
            _colorRoles[static_cast<size_t>(ColorRole::Blue)] = imaging::Color4f(.3F, .3F, .6F);
            _colorRoles[static_cast<size_t>(ColorRole::Cyan)] = imaging::Color4f(.3F, .6F, .6F);
            _colorRoles[static_cast<size_t>(ColorRole::Magenta)] = imaging::Color4f(.6F, .3F, .6F);
            _colorRoles[static_cast<size_t>(ColorRole::Yellow)] = imaging::Color4f(.6F, .6F, .3F);

            _fontRoles.resize(static_cast<size_t>(FontRole::Count));
            _fontRoles[static_cast<size_t>(FontRole::Label)] = imaging::FontInfo("NotoSans-Regular", 12 * 1);
            _fontRoles[static_cast<size_t>(FontRole::Mono)] = imaging::FontInfo("NotoMono-Regular", 12 * 1);
            _fontRoles[static_cast<size_t>(FontRole::Title)] = imaging::FontInfo("NotoSans-Bold", 12 * 1);

            p.changed = observer::Value<bool>::create();
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

        void Style::setSizeRole(SizeRole role, int value)
        {
            TLRENDER_P();
            if (_sizeRoles[static_cast<size_t>(role)] == value)
                return;
            _sizeRoles[static_cast<size_t>(role)] = value;
            p.changed->setAlways(true);
        }

        void Style::setColorRole(ColorRole role, const imaging::Color4f& value)
        {
            TLRENDER_P();
            if (_colorRoles[static_cast<size_t>(role)] == value)
                return;
            _colorRoles[static_cast<size_t>(role)] = value;
            p.changed->setAlways(true);
        }

        void Style::setFontRole(FontRole role, const imaging::FontInfo& value)
        {
            TLRENDER_P();
            if (_fontRoles[static_cast<size_t>(role)] == value)
                return;
            _fontRoles[static_cast<size_t>(role)] = value;
            p.changed->setAlways(true);
        }

        std::shared_ptr<observer::IValue<bool> > Style::observeChanged() const
        {
            return _p->changed;
        }
    }
}
