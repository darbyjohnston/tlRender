// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrCore/Math.h>

#include <sstream>

namespace tlr
{
    void App::_renderVideo()
    {
        if (_currentImage)
        {
            _render->drawImage(
                _currentImage,
                app::fitImageInWindow(_currentImage->getSize(), _frameBufferSize));
        }
    }

    void App::_renderHUD()
    {
        const uint16_t fontSize =
            math::clamp(
            ceilf(14 * _contentScale.y),
            0.F,
            static_cast<float>(std::numeric_limits<uint16_t>::max()));

        auto i = _hudLabels.find(app::HUDElement::UpperLeft);
        if (i != _hudLabels.end())
        {
            app::drawHUDLabel(
                _render,
                _fontSystem,
                _frameBufferSize,
                i->second,
                render::FontFamily::NotoSans,
                fontSize,
                app::HUDElement::UpperLeft);
        }

        i = _hudLabels.find(app::HUDElement::LowerLeft);
        if (i != _hudLabels.end())
        {
            app::drawHUDLabel(
                _render,
                _fontSystem,
                _frameBufferSize,
                i->second,
                render::FontFamily::NotoMono,
                fontSize,
                app::HUDElement::LowerLeft);
        }

        i = _hudLabels.find(app::HUDElement::LowerRight);
        if (i != _hudLabels.end())
        {
            app::drawHUDLabel(
                _render,
                _fontSystem,
                _frameBufferSize,
                i->second,
                render::FontFamily::NotoMono,
                fontSize,
                app::HUDElement::LowerRight);
        }
    }

    void App::_hudCallback(bool value)
    {
        _options.hud = value;
        {
            std::stringstream ss;
            ss << "HUD: " << _options.hud;
            _print(ss.str());
        }
    }
}
