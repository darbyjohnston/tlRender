// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlBakeApp/App.h>

#include <tlTimelineGL/Render.h>

#include <tlIO/System.h>

#include <tlCore/Time.h>

#include <feather-tk/gl/GL.h>
#include <feather-tk/gl/Util.h>
#include <feather-tk/gl/Window.h>
#include <feather-tk/core/CmdLine.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/Math.h>
#include <feather-tk/core/String.h>

namespace tl
{
    namespace bake
    {
        void App::_init(
            const std::shared_ptr<feather_tk::Context>& context,
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
                    feather_tk::CmdLineValueArg<std::string>::create(
                        _input,
                        "input",
                        "The input timeline."),
                    feather_tk::CmdLineValueArg<std::string>::create(
                        _output,
                        "output",
                        "The output file.")
                },
                {
                    feather_tk::CmdLineValueOption<OTIO_NS::TimeRange>::create(
                        _options.inOutRange,
                        { "-inOutRange" },
                        "Set the in/out points range."),
                    feather_tk::CmdLineValueOption<feather_tk::Size2I>::create(
                        _options.renderSize,
                        { "-renderSize", "-rs" },
                        "Render size."),
                    feather_tk::CmdLineValueOption<feather_tk::ImageType>::create(
                        _options.outputPixelType,
                        { "-outputPixelType", "-op" },
                        "Output pixel type.",
                        std::string(),
                        feather_tk::join(feather_tk::getImageTypeLabels(), ", ")),
                    feather_tk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.fileName,
                        { "-ocio" },
                        "OpenColorIO configuration file name (e.g., config.ocio)."),
                    feather_tk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.input,
                        { "-ocioInput" },
                        "OpenColorIO input name."),
                    feather_tk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.display,
                        { "-ocioDisplay" },
                        "OpenColorIO display name."),
                    feather_tk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.view,
                        { "-ocioView" },
                        "OpenColorIO view name."),
                    feather_tk::CmdLineValueOption<std::string>::create(
                        _options.ocioOptions.look,
                        { "-ocioLook" },
                        "OpenColorIO look name."),
                    feather_tk::CmdLineValueOption<std::string>::create(
                        _options.lutOptions.fileName,
                        { "-lut" },
                        "LUT file name."),
                    feather_tk::CmdLineValueOption<timeline::LUTOrder>::create(
                        _options.lutOptions.order,
                        { "-lutOrder" },
                        "LUT operation order.",
                        feather_tk::Format("{0}").arg(_options.lutOptions.order),
                        feather_tk::join(timeline::getLUTOrderLabels(), ", ")),
                    feather_tk::CmdLineValueOption<float>::create(
                        _options.sequenceDefaultSpeed,
                        { "-sequenceDefaultSpeed" },
                        "Default speed for image sequences.",
                        feather_tk::Format("{0}").arg(_options.sequenceDefaultSpeed)),
                    feather_tk::CmdLineValueOption<int>::create(
                        _options.sequenceThreadCount,
                        { "-sequenceThreadCount" },
                        "Number of threads for image sequence I/O.",
                        feather_tk::Format("{0}").arg(_options.sequenceThreadCount)),
#if defined(TLRENDER_EXR)
                    feather_tk::CmdLineValueOption<exr::Compression>::create(
                        _options.exrCompression,
                        { "-exrCompression" },
                        "OpenEXR output compression.",
                        feather_tk::Format("{0}").arg(_options.exrCompression),
                        feather_tk::join(exr::getCompressionLabels(), ", ")),
                    feather_tk::CmdLineValueOption<float>::create(
                        _options.exrDWACompressionLevel,
                        { "-exrDWACompressionLevel" },
                        "OpenEXR DWA compression level.",
                        feather_tk::Format("{0}").arg(_options.exrDWACompressionLevel)),
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
                    feather_tk::CmdLineValueOption<std::string>::create(
                        _options.ffmpegCodec,
                        { "-ffmpegCodec", "-ffc" },
                        "FFmpeg output codec.",
                        std::string(),
                        feather_tk::join(ffmpegCodecs, ", ")),
                    feather_tk::CmdLineValueOption<int>::create(
                        _options.ffmpegThreadCount,
                        { "-ffmpegThreadCount" },
                        "Number of threads for FFmpeg I/O.",
                        feather_tk::Format("{0}").arg(_options.ffmpegThreadCount)),
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
                    feather_tk::CmdLineValueOption<int>::create(
                        _options.usdRenderWidth,
                        { "-usdRenderWidth" },
                        "USD render width.",
                        feather_tk::Format("{0}").arg(_options.usdRenderWidth)),
                    feather_tk::CmdLineValueOption<float>::create(
                        _options.usdComplexity,
                        { "-usdComplexity" },
                        "USD render complexity setting.",
                        feather_tk::Format("{0}").arg(_options.usdComplexity)),
                    feather_tk::CmdLineValueOption<usd::DrawMode>::create(
                        _options.usdDrawMode,
                        { "-usdDrawMode" },
                        "USD draw mode.",
                        feather_tk::Format("{0}").arg(_options.usdDrawMode),
                        feather_tk::join(usd::getDrawModeLabels(), ", ")),
                    feather_tk::CmdLineValueOption<bool>::create(
                        _options.usdEnableLighting,
                        { "-usdEnableLighting" },
                        "USD enable lighting.",
                        feather_tk::Format("{0}").arg(_options.usdEnableLighting)),
                    feather_tk::CmdLineValueOption<bool>::create(
                        _options.usdSRGB,
                        { "-usdSRGB" },
                        "USD enable sRGB color space.",
                        feather_tk::Format("{0}").arg(_options.usdSRGB)),
                    feather_tk::CmdLineValueOption<size_t>::create(
                        _options.usdStageCache,
                        { "-usdStageCache" },
                        "USD stage cache size.",
                        feather_tk::Format("{0}").arg(_options.usdStageCache)),
                    feather_tk::CmdLineValueOption<size_t>::create(
                        _options.usdDiskCache,
                        { "-usdDiskCache" },
                        "USD disk cache size in gigabytes. A size of zero disables the cache.",
                        feather_tk::Format("{0}").arg(_options.usdDiskCache)),
#endif // TLRENDER_USD
                });
        }

        App::App()
        {}

        App::~App()
        {}

        std::shared_ptr<App> App::create(
            const std::shared_ptr<feather_tk::Context>& context,
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
            _window = feather_tk::gl::Window::create(
                _context,
                "tlbake",
                feather_tk::Size2I(1, 1),
                static_cast<int>(feather_tk::gl::WindowOptions::MakeCurrent));

            // Read the timeline.
            timeline::Options options;
            options.ioOptions = _getIOOptions();
            _timeline = timeline::Timeline::create(_context, _input, options);
            _timeRange = _timeline->getTimeRange();
            _print(feather_tk::Format("Timeline range: {0}-{1}").
                arg(_timeRange.start_time().value()).
                arg(_timeRange.end_time_inclusive().value()));
            _print(feather_tk::Format("Timeline speed: {0}").arg(_timeRange.duration().rate()));

            // Time range.
            if (time::isValid(_options.inOutRange))
            {
                _timeRange = _options.inOutRange;
            }
            _print(feather_tk::Format("In/out range: {0}-{1}").
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
                feather_tk::Size2I(info.video[0].size.w, info.video[0].size.h);
            _print(feather_tk::Format("Render size: {0}").arg(_renderSize));

            // Create the renderer.
            _render = timeline_gl::Render::create(_context);
            feather_tk::gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.color = feather_tk::gl::offscreenColorDefault;
            _buffer = feather_tk::gl::OffscreenBuffer::create(_renderSize, offscreenBufferOptions);

            // Create the writer.
            _writerPlugin = _context->getSystem<io::WriteSystem>()->getPlugin(file::Path(_output));
            if (!_writerPlugin)
            {
                throw std::runtime_error(feather_tk::Format("Cannot open: \"{0}\"").arg(_output));
            }
            _outputInfo.size.w = _renderSize.w;
            _outputInfo.size.h = _renderSize.h;
            _outputInfo.type = _options.outputPixelType != feather_tk::ImageType::None ?
                _options.outputPixelType :
                info.video[0].type;
            _outputInfo = _writerPlugin->getInfo(_outputInfo);
            if (feather_tk::ImageType::None == _outputInfo.type)
            {
                _outputInfo.type = feather_tk::ImageType::RGB_U8;
            }
            _print(feather_tk::Format("Output info: {0} {1}").
                arg(_outputInfo.size).
                arg(_outputInfo.type));
            _outputImage = feather_tk::Image::create(_outputInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(_outputInfo);
            ioInfo.videoTime = _timeRange;
            _writer = _writerPlugin->write(file::Path(_output), ioInfo, _getIOOptions());
            if (!_writer)
            {
                throw std::runtime_error(feather_tk::Format("Cannot open: \"{0}\"").arg(_output));
            }

            // Start the main loop.
            feather_tk::gl::OffscreenBufferBinding binding(_buffer);
            while (_running)
            {
                _tick();
            }

            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - _startTime;
            _print(feather_tk::Format("Seconds elapsed: {0}").arg(diff.count()));
            _print(feather_tk::Format("Average FPS: {0}").arg(_timeRange.duration().value() / diff.count()));
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
                ss << _options.usdDiskCache * feather_tk::gigabyte;
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
                { feather_tk::Box2I(0, 0, _renderSize.w, _renderSize.h) });
            _render->end();

            // Write the frame.
            glPixelStorei(GL_PACK_ALIGNMENT, _outputInfo.layout.alignment);
#if defined(FEATHER_TK_API_GL_4_1)
            glPixelStorei(GL_PACK_SWAP_BYTES, _outputInfo.layout.endian != feather_tk::getEndian());
#endif // FEATHER_TK_API_GL_4_1
            const GLenum format = feather_tk::gl::getReadPixelsFormat(_outputInfo.type);
            const GLenum type = feather_tk::gl::getReadPixelsType(_outputInfo.type);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(feather_tk::Format("Cannot open: \"{0}\"").arg(_output));
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
                _print(feather_tk::Format("Complete: {0}%").arg(static_cast<int>(c / static_cast<float>(d) * 100)));
            }
        }
    }
}
