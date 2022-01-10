// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrCore/AVIOSystem.h>
#include <tlrCore/File.h>
#include <tlrCore/Math.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

namespace tlr
{
    void App::_init(int argc, char* argv[])
    {
        IApp::_init(
            argc,
            argv,
            "tlrbake",
            "Convert an editorial timeline to a movie or image sequence.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "input",
                    "The input timeline."),
                app::CmdLineValueArg<std::string>::create(
                    _output,
                    "output",
                    "The output file.")
            },
            {
                app::CmdLineValueOption<int64_t>::create(
                    _options.startFrame,
                    { "-startFrame", "-sf" },
                    "Start frame."),
                app::CmdLineValueOption<int64_t>::create(
                    _options.endFrame,
                    { "-endFrame", "-ef" },
                    "End frame."),
                app::CmdLineValueOption<imaging::Size>::create(
                    _options.renderSize,
                    { "-renderSize", "-rs" },
                    "Render size."),
                app::CmdLineValueOption<imaging::PixelType>::create(
                    _options.outputPixelType,
                    { "-outputPixelType", "-op" },
                    "Output pixel type.",
                    std::string(),
                    string::join(imaging::getPixelTypeLabels(), ", ")),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.config,
                    { "-colorConfig", "-cc" },
                    "Color configuration file (config.ocio)."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.input,
                    { "-colorInput", "-ci" },
                    "Input color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.display,
                    { "-colorDisplay", "-cd" },
                    "Display color space."),
                app::CmdLineValueOption<std::string>::create(
                    _options.colorConfig.view,
                    { "-colorView", "-cv" },
                    "View color space.")
            });
    }

    App::App()
    {}

    App::~App()
    {}

    std::shared_ptr<App> App::create(int argc, char* argv[])
    {
        auto out = std::shared_ptr<App>(new App);
        out->_init(argc, argv);
        return out;
    }

    void App::run()
    {
        if (_exit != 0)
        {
            return;
        }

        _startTime = std::chrono::steady_clock::now();
        
        // Read the timeline.
        _timeline = timeline::Timeline::create(_input, _context);
        _duration = _timeline->getDuration();
        _print(string::Format("Timeline duration: {0}").arg(_duration.value()));
        _print(string::Format("Timeline speed: {0}").arg(_duration.rate()));

        // Time range.
        auto startTime = _options.startFrame >= 0 ?
            otime::RationalTime(_options.startFrame, _duration.rate()) :
            otime::RationalTime(0, _duration.rate());
        _range = _options.endFrame >= 0 ?
            otime::TimeRange::range_from_start_end_time_inclusive(startTime, otime::RationalTime(_options.endFrame, _duration.rate())) :
            otime::TimeRange::range_from_start_end_time(startTime, startTime + _duration);
        _currentTime = _range.start_time();
        _print(string::Format("Frame range: {0}-{1}").arg(_range.start_time().value()).arg(_range.end_time_inclusive().value()));

        // Render information.
        const auto& info = _timeline->getAVInfo();
        if (info.video.empty())
        {
            throw std::runtime_error("No video information");
        }
        _renderSize = _options.renderSize.isValid() ?
            _options.renderSize :
            info.video[0].size;
        _print(string::Format("Render size: {0}").arg(_renderSize));

        // Create the renderer.
        _fontSystem = imaging::FontSystem::create();
        _render = render::SoftwareRender::create(_context);

        // Create the writer.
        _writerPlugin = _context->getSystem<avio::System>()->getPlugin(file::Path(_output));
        if (!_writerPlugin)
        {
            throw std::runtime_error(string::Format("{0}: Cannot open").arg(_output));
        }
        avio::Info ioInfo;
        _outputInfo.size = _renderSize;
        const auto timelinePixelType = imaging::PixelType::YUV_420P == info.video[0].pixelType ?
            imaging::PixelType::RGB_U8 :
            info.video[0].pixelType;
        const auto writePixelTypes = _writerPlugin->getWritePixelTypes();
        _outputInfo.pixelType = _options.outputPixelType != imaging::PixelType::None ?
            _options.outputPixelType :
            (!writePixelTypes.empty() ? imaging::getClosest(timelinePixelType, writePixelTypes) : timelinePixelType);
        _outputInfo.layout.alignment = _writerPlugin->getWriteAlignment(_outputInfo.pixelType);
        _outputInfo.layout.endian = _writerPlugin->getWriteEndian();
        _print(string::Format("Output info: {0}").arg(_outputInfo));
        _outputImage = imaging::Image::create(_outputInfo);
        ioInfo.video.push_back(_outputInfo);
        ioInfo.videoTime = _range;
        _writer = _writerPlugin->write(file::Path(_output), ioInfo);
        if (!_writer)
        {
            throw std::runtime_error(string::Format("{0}: Cannot open").arg(_output));
        }

        // Start the main loop.
        while (_running)
        {
            _tick();
        }

        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> diff = now - _startTime;
        _print(string::Format("Seconds elapsed: {0}").arg(diff.count()));
        _print(string::Format("Average FPS: {0}").arg(_range.duration().value() / diff.count()));
    }

    void App::_tick()
    {
        _printProgress();

        // Set the active range.
        _timeline->setActiveRanges({ otime::TimeRange(
            _timeline->getGlobalStartTime() + _currentTime,
            otime::RationalTime(1.0, _currentTime.rate())) });

        // Render the video.
        _render->setColorConfig(_options.colorConfig);
        _render->begin(_renderSize);
        const auto videoData = _timeline->getVideo(_timeline->getGlobalStartTime() + _currentTime).get();
        _render->drawVideo(videoData);
        _render->end();

        // Write the frame.
        _writer->writeVideo(_currentTime, _render->copyFrameBuffer(_outputInfo.pixelType));

        // Advance the time.
        _currentTime += otime::RationalTime(1, _currentTime.rate());
        if (_currentTime > _range.end_time_inclusive())
        {
            _running = false;
        }
    }

    void App::_printProgress()
    {
        const int64_t c = static_cast<int64_t>(_currentTime.value() - _range.start_time().value());
        const int64_t d = static_cast<int64_t>(_range.duration().value());
        if (d >= 100 && c % (d / 100) == 0)
        {
            _print(string::Format("Complete: {0}%").arg(static_cast<int>(c / static_cast<float>(d) * 100)));
        }
    }
}
