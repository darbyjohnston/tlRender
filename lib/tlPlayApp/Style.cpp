// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Style.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace play_app
    {
        DTK_ENUM_IMPL(
            StylePalette,
            "Dark",
            "Light");

        std::map<ui::ColorRole, dtk::Color4F> getStylePalette(StylePalette value)
        {
            auto out = ui::defaultColorRoles();
            switch (value)
            {
            case StylePalette::Light:
                out[ui::ColorRole::Window] = dtk::Color4F(.9F, .9F, .9F);
                out[ui::ColorRole::Base] = dtk::Color4F(1.F, 1.F, 1.F);
                out[ui::ColorRole::Button] = dtk::Color4F(.8F, .8F, .8F);
                out[ui::ColorRole::Text] = dtk::Color4F(0.F, 0.F, 0.F);
                out[ui::ColorRole::TextDisabled] = dtk::Color4F(.5F, .5F, .5F);
                out[ui::ColorRole::Border] = dtk::Color4F(.6F, .6F, .6F);
                out[ui::ColorRole::Hover] = dtk::Color4F(0.F, 0.F, 0.F, .1F);
                out[ui::ColorRole::Pressed] = dtk::Color4F(0.F, 0.F, 0.F, .2F);
                out[ui::ColorRole::Checked] = dtk::Color4F(.6F, .7F, 1.F);
                out[ui::ColorRole::KeyFocus] = dtk::Color4F(.3F, .4F, 1.F);

                out[ui::ColorRole::InOut] = dtk::Color4F(.4F, .5F, .9F);
                out[ui::ColorRole::VideoCache] = dtk::Color4F(.3F, .7F, .7F);
                out[ui::ColorRole::AudioCache] = dtk::Color4F(.5F, .3F, .7F);
                out[ui::ColorRole::VideoClip] = dtk::Color4F(.5F, .7F, .7F);
                out[ui::ColorRole::VideoGap] = dtk::Color4F(.55F, .61F, .61F);
                out[ui::ColorRole::AudioClip] = dtk::Color4F(.6F, .55F, .7F);
                out[ui::ColorRole::AudioGap] = dtk::Color4F(.55F, .54F, .6F);
                out[ui::ColorRole::Transition] = dtk::Color4F(.7F, .6F, .6F);
                break;
            default: break;
            }
            return out;
        }
    }
}
