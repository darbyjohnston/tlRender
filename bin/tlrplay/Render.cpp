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
        for (auto i : _readers)
        {
            otio::ErrorStatus errorStatus;
            auto range = i.first.value->trimmed_range_in_parent(&errorStatus);
            if (errorStatus != otio::ErrorStatus::OK)
            {
                throw std::runtime_error(errorStatus.full_description);
            }
            if (range.has_value())
            {
                // Is the clip active?
                if (_currentTime >= range.value().start_time() &&
                    _currentTime < range.value().start_time() + range.value().duration())
                {
                    auto& queue = i.second->getVideoQueue();
                    if (queue.size())
                    {
                        // Get the frame from the video queue, discarding out of date frames.
                        av::io::VideoFrame frame = queue.front();
                        auto time = i.first.value->transformed_time(frame.time, _flattenedTimeline, &errorStatus);
                        if (errorStatus != otio::ErrorStatus::OK)
                        {
                            throw std::runtime_error(errorStatus.full_description);
                        }
                        while (queue.size() > 1 && time < _currentTime)
                        {
                            queue.pop();
                            frame = queue.front();
                            time = i.first.value->transformed_time(frame.time, _flattenedTimeline, &errorStatus);
                            if (errorStatus != otio::ErrorStatus::OK)
                            {
                                throw std::runtime_error(errorStatus.full_description);
                            }
                        }

                        // Draw the image.
                        if (const auto& image = frame.image)
                        {
                            _render->drawImage(
                                image, 
                                app::fitImageInWindow(image->getSize(), _frameBufferSize));
                        }
                    }
                }
            }
        }
    }

    void App::_renderHUD()
    {
        const uint16_t fontSize =
            math::clamp(
            ceilf(14 * _contentScale.y),
            0.F,
            static_cast<float>(std::numeric_limits<uint16_t>::max()));

        // Draw the input file name.
        app::drawHUDLabel(
            _render,
            _fontSystem,
            _frameBufferSize,
            "Input: " + _input,
            render::FontFamily::NotoSans,
            fontSize,
            app::HUDElement::UpperLeft);

        // Draw the current time.
        otime::ErrorStatus errorStatus;
        const std::string label = _currentTime.to_timecode(&errorStatus);
        if (errorStatus != otio::ErrorStatus::OK)
        {
            throw std::runtime_error(errorStatus.details);
        }
        app::drawHUDLabel(
            _render,
            _fontSystem,
            _frameBufferSize,
            "Time: " + label,
            render::FontFamily::NotoMono,
            fontSize,
            app::HUDElement::LowerLeft);

        // Draw the speed.
        std::stringstream ss;
        ss.precision(2);
        ss << "Speed: " << std::fixed << _duration.rate();
        app::drawHUDLabel(
            _render,
            _fontSystem,
            _frameBufferSize,
            ss.str(),
            render::FontFamily::NotoMono,
            fontSize,
            app::HUDElement::LowerRight);
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
