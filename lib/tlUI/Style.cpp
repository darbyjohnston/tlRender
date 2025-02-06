// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/Style.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <algorithm>
#include <sstream>

namespace tl
{
    namespace ui
    {
        DTK_ENUM_IMPL(
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
            "HandleSmall",
            "Swatch",
            "SwatchLarge",
            "Shadow",
            "DragLength");

        std::map<SizeRole, int> defaultSizeRoles()
        {
            std::map<SizeRole, int> out;
            out[SizeRole::Margin] = 10;
            out[SizeRole::MarginSmall] = 5;
            out[SizeRole::MarginLarge] = 20;
            out[SizeRole::MarginInside] = 2;
            out[SizeRole::MarginDialog] = 40;
            out[SizeRole::Spacing] = 10;
            out[SizeRole::SpacingSmall] = 5;
            out[SizeRole::SpacingLarge] = 20;
            out[SizeRole::SpacingTool] = 2;
            out[SizeRole::Border] = 1;
            out[SizeRole::ScrollArea] = 200;
            out[SizeRole::Slider] = 100;
            out[SizeRole::Handle] = 8;
            out[SizeRole::HandleSmall] = 6;
            out[SizeRole::Swatch] = 20;
            out[SizeRole::SwatchLarge] = 40;
            out[SizeRole::Shadow] = 15;
            out[SizeRole::DragLength] = 10;
            return out;
        }

        DTK_ENUM_IMPL(
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
            "ToolTipWindow",
            "ToolTipText",

            "InOut",
            "FrameMarker",
            "VideoCache",
            "AudioCache",
            "VideoClip",
            "VideoGap",
            "AudioClip",
            "AudioGap",
            "Transition",

            "Red",
            "Green",
            "Blue",
            "Cyan",
            "Magenta",
            "Yellow");

        std::map<ColorRole, dtk::Color4F> defaultColorRoles()
        {
            std::map<ColorRole, dtk::Color4F> out;
            out[ColorRole::None] = dtk::Color4F();

            out[ColorRole::Window] = dtk::Color4F(.2F, .2F, .2F);
            out[ColorRole::Base] = dtk::Color4F(.17F, .17F, .17F);
            out[ColorRole::Button] = dtk::Color4F(.3F, .3F, .3F);
            out[ColorRole::Text] = dtk::Color4F(1.F, 1.F, 1.F);
            out[ColorRole::TextDisabled] = dtk::Color4F(.5F, .5F, .5F);
            out[ColorRole::Border] = dtk::Color4F(.13F, .13F, .13F);
            out[ColorRole::Hover] = dtk::Color4F(1.F, 1.F, 1.F, .1F);
            out[ColorRole::Pressed] = dtk::Color4F(1.F, 1.F, 1.F, .2F);
            out[ColorRole::Checked] = dtk::Color4F(.6F, .4F, .2F);
            out[ColorRole::KeyFocus] = dtk::Color4F(.6F, .6F, .4F);
            out[ColorRole::Overlay] = dtk::Color4F(0.F, 0.F, 0.F, .5F);
            out[ColorRole::ToolTipWindow] = dtk::Color4F(1.F, .95F, .7F);
            out[ColorRole::ToolTipText] = dtk::Color4F(0.F, 0.F, 0.F);

            out[ColorRole::InOut] = dtk::Color4F(1.F, .7F, .2F, .1F);
            out[ColorRole::FrameMarker] = dtk::Color4F(.6F, .4F, .2F);
            out[ColorRole::VideoCache] = dtk::Color4F(.2F, .4F, .4F);
            out[ColorRole::AudioCache] = dtk::Color4F(.3F, .25F, .4F);
            out[ColorRole::VideoClip] = dtk::Color4F(.2F, .4F, .4F);
            out[ColorRole::VideoGap] = dtk::Color4F(.25F, .31F, .31F);
            out[ColorRole::AudioClip] = dtk::Color4F(.3F, .25F, .4F);
            out[ColorRole::AudioGap] = dtk::Color4F(.25F, .24F, .3F);
            out[ColorRole::Transition] = dtk::Color4F(.4F, .3F, .3F);

            out[ColorRole::Red] = dtk::Color4F(.6F, .3F, .3F);
            out[ColorRole::Green] = dtk::Color4F(.3F, .6F, .3F);
            out[ColorRole::Blue] = dtk::Color4F(.3F, .3F, .6F);
            out[ColorRole::Cyan] = dtk::Color4F(.3F, .6F, .6F);
            out[ColorRole::Magenta] = dtk::Color4F(.6F, .3F, .6F);
            out[ColorRole::Yellow] = dtk::Color4F(.6F, .6F, .3F);
            return out;
        }

        DTK_ENUM_IMPL(
            FontRole,
            "None",
            "Label",
            "Mono",
            "Title");

        std::map<FontRole, dtk::FontInfo> defaultFontRoles()
        {
            std::map<FontRole, dtk::FontInfo> out;
            out[FontRole::Label] = dtk::FontInfo("NotoSans-Regular", 12 * 1);
            out[FontRole::Mono] = dtk::FontInfo("NotoSansMono-Regular", 12 * 1);
            out[FontRole::Title] = dtk::FontInfo("NotoSans-Regular", 16 * 1);
            return out;
        }

        struct Style::Private
        {
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<dtk::ObservableValue<bool> > changed;
        };

        void Style::_init(
            const std::shared_ptr<dtk::Context>& context)
        {
            DTK_P();
            p.context = context;
            _sizeRoles = defaultSizeRoles();
            _colorRoles = defaultColorRoles();
            _fontRoles = defaultFontRoles();
            p.changed = dtk::ObservableValue<bool>::create();
        }

        Style::Style() :
            _p(new Private)
        {}

        Style::~Style()
        {}

        std::shared_ptr<Style> Style::create(
            const std::shared_ptr<dtk::Context>& context)
        {
            auto out = std::shared_ptr<Style>(new Style);
            out->_init(context);
            return out;
        }

        void Style::setSizeRole(SizeRole role, int value)
        {
            DTK_P();
            if (_sizeRoles[role] == value)
                return;
            _sizeRoles[role] = value;
            p.changed->setAlways(true);
        }

        void Style::setSizeRoles(const std::map<SizeRole, int>& value)
        {
            DTK_P();
            if (_sizeRoles == value)
                return;
            _sizeRoles = value;
            p.changed->setAlways(true);
        }

        void Style::setColorRole(ColorRole role, const dtk::Color4F& value)
        {
            DTK_P();
            if (_colorRoles[role] == value)
                return;
            _colorRoles[role] = value;
            p.changed->setAlways(true);
        }

        void Style::setColorRoles(const std::map<ColorRole, dtk::Color4F>& value)
        {
            DTK_P();
            if (_colorRoles == value)
                return;
            _colorRoles = value;
            p.changed->setAlways(true);
        }

        void Style::setFontRole(FontRole role, const dtk::FontInfo& value)
        {
            DTK_P();
            if (_fontRoles[role] == value)
                return;
            _fontRoles[role] = value;
            p.changed->setAlways(true);
        }

        void Style::setFontRoles(const std::map<FontRole, dtk::FontInfo>& value)
        {
            DTK_P();
            if (_fontRoles == value)
                return;
            _fontRoles = value;
            p.changed->setAlways(true);
        }

        std::shared_ptr<dtk::IObservableValue<bool> > Style::observeChanged() const
        {
            return _p->changed;
        }

        void to_json(nlohmann::json& json, const std::map<ColorRole, dtk::Color4F>& in)
        {
            std::map<std::string, std::string> s;
            for (const auto& i : in)
            {
                std::stringstream ss;
                ss << i.first;
                std::stringstream ss2;
                ss2 << i.second;
                s[ss.str()] = ss2.str();
            }
            json = s;
        }

        void from_json(const nlohmann::json& json, std::map<ColorRole, dtk::Color4F>& out)
        {
            for (auto i = json.begin(); i != json.end(); ++i)
            {
                std::stringstream ss(i.key());
                ColorRole colorRole = ColorRole::None;
                ss >> colorRole;
                std::stringstream ss2(std::string(i.value()));
                dtk::Color4F color;
                ss2 >> color;
                out[colorRole] = color;
            }
        }
    }
}
