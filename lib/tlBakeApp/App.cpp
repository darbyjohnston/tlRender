// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlBakeApp/App.h>

#include <tlTimelineGL/Render.h>

#include <tlIO/System.h>

#include <tlCore/Time.h>

#include <dtk/gl/GL.h>
#include <dtk/gl/Util.h>
#include <dtk/gl/Window.h>
#include <dtk/core/CmdLine.h>
#include <dtk/core/Context.h>
#include <dtk/core/Format.h>
#include <dtk/core/Math.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace bake
    {
        void App::_init(
            const std::shared_ptr<dtk::Context>& context,
            std::vector<std::string>& argv)
        {
            std::vector<std::string> ffmpegCodecs;
#if defined(TLRENDER_FFMPEG)
            auto ioSystem = context->getSystem<io::WriteSystem>();
            auto ffmpegPlugin = ioSystem->getPlugin<ffmpeg::WritePlugin>();
            ffmpegCodecs = ffmpegPlugin->getCodecs();
#endif // TLRENDER_FFMPEG
            IApp::_init(
                context,
                argv,
                "tlbake",
                "Render a timeline to a movie or image sequence.",
                {
                    dtk::CmdLineValueArg<std::string>::create(
                        _input,
                        "input",
                        "The input timeline."),
                    dtk::CmdLineValueArg<std::string>::create(
                        _output,
                        "output",
                        "The output file.")
                },
                {
                    dtk::CmdLineValueOption<OTIO_NS::TimeRange>::create(
                        _options.inOutRange,
                        { "-inOutRange" },
                        "Set the in/out points range."),
                    dtk::CmdLineValueOption<dtk::Size2I>::create(
                        _options.renderSize,
                        { "-renderSize", "-rs" },
                        "Render size."),
                    dtk::CmdLineValueOption<dtk::ImageType>::create(
                        _options.outputPixelType,
                        { "-outputPixelType", "-op" },
                        "Output pixel type.",
                        std::string(),
                        dtk::join(dtk::getImageTypeLabels(), ", ")),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.fileName,
                        { "-ocio" },
                        "OpenColorIO configuration file name (e.g., config.ocio)."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.input,
                        { "-ocioInput" },
                        "OpenColorIO input name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.display,
                        { "-ocioDisplay" },
                        "OpenColorIO display name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.view,
                        { "-ocioView" },
                        "OpenColorIO view name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.look,
                        { "-ocioLook" },
                        "OpenColorIO look name."),
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.lutOptions.fileName,
                        { "-lut" },
                        "LUT file name."),
                    dtk::CmdLineValueOption<timeline::LUTOrder>::create(
                        _options.lutOptions.order,
                        { "-lutOrder" },
                        "LUT operation order.",
                        dtk::Format("{0}").arg(_options.lutOptions.order),
                        dtk::join(timeline::getLUTOrderLabels(), ", ")),
                    dtk::CmdLineValueOption<float>::create(
                        _options.sequenceDefaultSpeed,
                        { "-sequenceDefaultSpeed" },
                        "Default speed for image sequences.",
                        dtk::Format("{0}").arg(_options.sequenceDefaultSpeed)),
                    dtk::CmdLineValueOption<int>::create(
                        _options.sequenceThreadCount,
                        { "-sequenceThreadCount" },
                        "Number of threads for image sequence I/O.",
                        dtk::Format("{0}").arg(_options.sequenceThreadCount)),
#if defined(TLRENDER_EXR)
                    dtk::CmdLineValueOption<exr::Compression>::create(
                        _options.exrCompression,
                        { "-exrCompression" },
                        "OpenEXR output compression.",
                        dtk::Format("{0}").arg(_options.exrCompression),
                        dtk::join(exr::getCompressionLabels(), ", ")),
                    dtk::CmdLineValueOption<float>::create(
                        _options.exrDWACompressionLevel,
                        { "-exrDWACompressionLevel" },
                        "OpenEXR DWA compression level.",
                        dtk::Format("{0}").arg(_options.exrDWACompressionLevel)),
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
                    dtk::CmdLineValueOption<std::string>::create(
                        _options.ffmpegCodec,
                        { "-ffmpegCodec", "-ffc" },
                        "FFmpeg output codec.",
                        std::string(),
                        dtk::join(ffmpegCodecs, ", ")),
                    dtk::CmdLineValueOption<int>::create(
                        _options.ffmpegThreadCount,
                        { "-ffmpegThreadCount" },
                        "Number of threads for FFmpeg I/O.",
                        dtk::Format("{0}").arg(_options.ffmpegThreadCount)),
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
                    dtk::CmdLineValueOption<int>::create(
                        _options.usdRenderWidth,
                        { "-usdRenderWidth" },
                        "USD render width.",
                        dtk::Format("{0}").arg(_options.usdRenderWidth)),
                    dtk::CmdLineValueOption<float>::create(
                        _options.usdComplexity,
                        { "-usdComplexity" },
                        "USD render complexity setting.",
                        dtk::Format("{0}").arg(_options.usdComplexity)),
                    dtk::CmdLineValueOption<usd::DrawMode>::create(
                        _options.usdDrawMode,
                        { "-usdDrawMode" },
                        "USD draw mode.",
                        dtk::Format("{0}").arg(_options.usdDrawMode),
                        dtk::join(usd::getDrawModeLabels(), ", ")),
                    dtk::CmdLineValueOption<bool>::create(
                        _options.usdEnableLighting,
                        { "-usdEnableLighting" },
                        "USD enable lighting.",
                        dtk::Format("{0}").arg(_options.usdEnableLighting)),
                    dtk::CmdLineValueOption<bool>::create(
                        _options.usdSRGB,
                        { "-usdSRGB" },
                        "USD enable sRGB color space.",
                        dtk::Format("{0}").arg(_options.usdSRGB)),
                    dtk::CmdLineValueOption<size_t>::create(
                        _options.usdStageCache,
                        { "-usdStageCache" },
                        "USD stage cache size.",
                        dtk::Format("{0}").arg(_options.usdStageCache)),
                    dtk::CmdLineValueOption<size_t>::create(
                        _options.usdDiskCache,
                        { "-usdDiskCache" },
                        "USD disk cache size in gigabytes. A size of zero disables the cache.",
                        dtk::Format("{0}").arg(_options.usdDiskCache)),
#endif // TLRENDER_USD
                });
        }

        App::App()
        {}

        App::~App()
        {}

        std::shared_ptr<App> App::create(
            const std::shared_ptr<dtk::Context>& context,
            std::vector<std::string>& argv)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(context, argv);
            return out;
        }

        void App::run()
        {
            _startTime = std::chrono::steady_clock::now();

            // Create the window.
            _window = dtk::gl::Window::create(
                _context,
                "tlbake",
                dtk::Size2I(1, 1),
                static_cast<int>(dtk::gl::WindowOptions::MakeCurrent));

            // Read the timeline.
            timeline::Options options;
            options.ioOptions = _getIOOptions();
            _timeline = timeline::Timeline::create(_context, _input, options);
            _timeRange = _timeline->getTimeRange();
            _print(dtk::Format("Timeline range: {0}-{1}").
                arg(_timeRange.start_time().value()).
                arg(_timeRange.end_time_inclusive().value()));
            _print(dtk::Format("Timeline speed: {0}").arg(_timeRange.duration().rate()));

            // Time range.
            if (time::isValid(_options.inOutRange))
            {
                _timeRange = _options.inOutRange;
            }
            _print(dtk::Format("In/out range: {0}-{1}").
                arg(_timeRange.start_time().value()).
                arg(_timeRange.end_time_inclusive().value()));
            _inputTime = _timeRange.start_time();
            _outputTime = OTIO_NS::RationalTime(0.0, _timeRange.duration().rate());

            // Render information.
            const auto& info = _timeline->getIOInfo();
            if (info.video.empty())
            {
                throw std::runtime_error("No video to render");
            }
            _renderSize = _options.renderSize.isValid() ?
                _options.renderSize :
                dtk::Size2I(info.video[0].size.w, info.video[0].size.h);
            _print(dtk::Format("Render size: {0}").arg(_renderSize));

            // Create the renderer.
            _render = timeline_gl::Render::create(_context);
            dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.color = dtk::gl::offscreenColorDefault;
            _buffer = dtk::gl::OffscreenBuffer::create(_renderSize, offscreenBufferOptions);

            // Create the writer.
            _writerPlugin = _context->getSystem<io::WriteSystem>()->getPlugin(file::Path(_output));
            if (!_writerPlugin)
            {
                throw std::runtime_error(dtk::Format("Cannot open: \"{0}\"").arg(_output));
            }
            _outputInfo.size.w = _renderSize.w;
            _outputInfo.size.h = _renderSize.h;
            _outputInfo.type = _options.outputPixelType != dtk::ImageType::None ?
                _options.outputPixelType :
                info.video[0].type;
            _outputInfo = _writerPlugin->getInfo(_outputInfo);
            if (dtk::ImageType::None == _outputInfo.type)
            {
                _outputInfo.type = dtk::ImageType::RGB_U8;
            }
            _print(dtk::Format("Output info: {0} {1}").
                arg(_outputInfo.size).
                arg(_outputInfo.type));
            _outputImage = dtk::Image::create(_outputInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(_outputInfo);
            ioInfo.videoTime = _timeRange;
            _writer = _writerPlugin->write(file::Path(_output), ioInfo, _getIOOptions());
            if (!_writer)
            {
                throw std::runtime_error(dtk::Format("Cannot open: \"{0}\"").arg(_output));
            }

            // Start the main loop.
            dtk::gl::OffscreenBufferBinding binding(_buffer);
            while (_running)
            {
                _tick();
            }

            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - _startTime;
            _print(dtk::Format("Seconds elapsed: {0}").arg(diff.count()));
            _print(dtk::Format("Average FPS: {0}").arg(_timeRange.duration().value() / diff.count()));
        }

        io::Options App::_getIOOptions() const
        {
            io::Options out;
            {
                std::stringstream ss;
                ss << _options.sequenceDefaultSpeed;
                out["SequenceIO/DefaultSpeed"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.sequenceThreadCount;
                out["SequenceIO/ThreadCount"] = ss.str();
            }

#if defined(TLRENDER_EXR)
            {
                std::stringstream ss;
                ss << _options.exrCompression;
                out["OpenEXR/Compression"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.exrDWACompressionLevel;
                out["OpenEXR/DWACompressionLevel"] = ss.str();
            }
#endif // TLRENDER_EXR

#if defined(TLRENDER_FFMPEG)
            if (!_options.ffmpegCodec.empty())
            {
                out["FFmpeg/Codec"] = _options.ffmpegCodec;
            }
            {
                std::stringstream ss;
                ss << _options.ffmpegThreadCount;
                out["FFmpeg/ThreadCount"] = ss.str();
            }
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            {
                std::stringstream ss;
                ss << _options.usdRenderWidth;
                out["USD/RenderWidth"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdComplexity;
                out["USD/Complexity"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdDrawMode;
                out["USD/DrawMode"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdEnableLighting;
                out["USD/EnableLighting"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdSRGB;
                out["USD/sRGB"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdStageCache;
                out["USD/StageCacheCount"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdDiskCache * dtk::gigabyte;
                out["USD/DiskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD

            return out;
        }

        void App::_tick()
        {
            _context->tick();

            _printProgress();

            // Render the video.
            _render->begin(_renderSize);
            _render->setOCIOOptions(_options.ocioOptions);
            _render->setLUTOptions(_options.lutOptions);
            const auto videoData = _timeline->getVideo(_inputTime).future.get();
            _render->drawVideo(
                { videoData },
                { dtk::Box2I(0, 0, _renderSize.w, _renderSize.h) });
            _render->end();

            // Write the frame.
            glPixelStorei(GL_PACK_ALIGNMENT, _outputInfo.layout.alignment);
#if defined(dtk_API_GL_4_1)
            glPixelStorei(GL_PACK_SWAP_BYTES, _outputInfo.layout.endian != dtk::getEndian());
#endif // dtk_API_GL_4_1
            const GLenum format = dtk::gl::getReadPixelsFormat(_outputInfo.type);
            const GLenum type = dtk::gl::getReadPixelsType(_outputInfo.type);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(dtk::Format("Cannot open: \"{0}\"").arg(_output));
            }
            glReadPixels(
                0,
                0,
                _outputInfo.size.w,
                _outputInfo.size.h,
                format,
                type,
                _outputImage->getData());
            _writer->writeVideo(_outputTime, _outputImage);

            // Advance the time.
            _inputTime += OTIO_NS::RationalTime(1, _inputTime.rate());
            if (_inputTime > _timeRange.end_time_inclusive())
            {
                _running = false;
            }
            _outputTime += OTIO_NS::RationalTime(1, _outputTime.rate());
        }

        void App::_printProgress()
        {
            const int64_t c = static_cast<int64_t>(_inputTime.value() - _timeRange.start_time().value());
            const int64_t d = static_cast<int64_t>(_timeRange.duration().value());
            if (d >= 100 && c % (d / 100) == 0)
            {
                _print(dtk::Format("Complete: {0}%").arg(static_cast<int>(c / static_cast<float>(d) * 100)));
            }
        }
    }
}
