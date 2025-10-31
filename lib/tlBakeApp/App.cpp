// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlBakeApp/App.h>

#include <tlTimelineGL/Render.h>

#include <tlIO/System.h>

#include <tlCore/Time.h>

#include <ftk/GL/GL.h>
#include <ftk/GL/Util.h>
#include <ftk/GL/Window.h>
#include <ftk/Core/CmdLine.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/Math.h>
#include <ftk/Core/String.h>

namespace tl
{
    namespace bake
    {
        void App::_init(
            const std::shared_ptr<ftk::Context>& context,
            std::vector<std::string>& argv)
        {
            std::vector<std::string> ffmpegCodecs;
#if defined(TLRENDER_FFMPEG)
            auto ioSystem = context->getSystem<io::WriteSystem>();
            auto ffmpegPlugin = ioSystem->getPlugin<ffmpeg::WritePlugin>();
            ffmpegCodecs = ffmpegPlugin->getCodecs();
#endif // TLRENDER_FFMPEG
            _cmdLine.input = ftk::CmdLineValueArg<std::string>::create(
                "input",
                "The input timeline.");
            _cmdLine.output = ftk::CmdLineValueArg<std::string>::create(
                "output",
                "The output file.");
            _cmdLine.inOutRange = ftk::CmdLineValueOption<OTIO_NS::TimeRange>::create(
                { "-inOutRange" },
                "Set the in/out points range.",
                "Render");
            _cmdLine.renderSize = ftk::CmdLineValueOption<ftk::Size2I>::create(
                { "-renderSize", "-rs" },
                "Render size.",
                "Render");
            _cmdLine.outputPixelType = ftk::CmdLineValueOption<ftk::ImageType>::create(
                { "-outputPixelType", "-op" },
                "Output pixel type.",
                "Render",
                std::optional<ftk::ImageType>(),
                ftk::quotes(ftk::getImageTypeLabels()));
            _cmdLine.ocioFileName = ftk::CmdLineValueOption<std::string>::create(
                { "-ocio" },
                "OCIO configuration file name (e.g., config.ocio).",
                "Color");
            _cmdLine.ocioInput = ftk::CmdLineValueOption<std::string>::create(
                { "-ocioInput" },
                "OCIO input name.",
                "Color");
            _cmdLine.ocioDisplay = ftk::CmdLineValueOption<std::string>::create(
                { "-ocioDisplay" },
                "OCIO display namee.",
                "Color");
            _cmdLine.ocioView = ftk::CmdLineValueOption<std::string>::create(
                { "-ocioView" },
                "OCIO view namee.",
                "Color");
            _cmdLine.ocioLook = ftk::CmdLineValueOption<std::string>::create(
                { "-ocioLook" },
                "OCIO look namee.",
                "Color");
            _cmdLine.lutFileName = ftk::CmdLineValueOption<std::string>::create(
                { "-lut" },
                "LUT file name.",
                "Color");
            _cmdLine.lutOrder = ftk::CmdLineValueOption<timeline::LUTOrder>::create(
                { "-lutOrder" },
                "LUT operation order.",
                "Color",
                std::optional<timeline::LUTOrder>(),
                ftk::quotes(timeline::getLUTOrderLabels()));
            _cmdLine.sequenceDefaultSpeed = ftk::CmdLineValueOption<double>::create(
                { "-sequenceDefaultSpeed" },
                "Default speed for image sequences.",
                "Image Sequences",
                io::SequenceOptions().defaultSpeed);
            _cmdLine.sequenceThreadCount = ftk::CmdLineValueOption<int>::create(
                { "-sequenceThreadCount" },
                "Number of threads for image sequence I/O.",
                "Image Sequences",
                static_cast<int>(io::SequenceOptions().threadCount));
#if defined(TLRENDER_EXR)
            _cmdLine.exrCompression = ftk::CmdLineValueOption<exr::Compression>::create(
                { "-exrCompression" },
                "Output compression.",
                "OpenEXR",
                exr::Compression::ZIP,
                ftk::quotes(exr::getCompressionLabels()));
            _cmdLine.exrDWACompressionLevel = ftk::CmdLineValueOption<float>::create(
                { "-exrDWACompressionLevel" },
                "DWA compression level.",
                "OpenEXR",
                45.F);
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
            _cmdLine.ffmpegCodec = ftk::CmdLineValueOption<std::string>::create(
                { "-ffmpegCodec", "-ffc" },
                "Output codec.",
                "FFmpeg",
                "mjpeg",
                ftk::quotes(ffmpegCodecs));
            _cmdLine.ffmpegThreadCount = ftk::CmdLineValueOption<int>::create(
                { "-ffmpegThreadCount" },
                "Number of threads for I/O.",
                "FFmpeg",
                static_cast<int>(ffmpeg::Options().threadCount));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            _cmdLine.usdRenderWidth = ftk::CmdLineValueOption<int>::create(
                { "-usdRenderWidth" },
                "Render width.",
                "USD",
                1920);
            _cmdLine.usdComplexity = ftk::CmdLineValueOption<float>::create(
                { "-usdComplexity" },
                "Render complexity setting.",
                "USD",
                1.F);
            _cmdLine.usdDrawMode = ftk::CmdLineValueOption<usd::DrawMode>::create(
                { "-usdDrawMode" },
                "Draw mode.",
                "USD",
                usd::DrawMode::ShadedSmooth,
                ftk::quotes(usd::getDrawModeLabels()));
            _cmdLine.usdEnableLighting = ftk::CmdLineValueOption<bool>::create(
                { "-usdEnableLighting" },
                "Enable lighting.",
                "USD",
                true);
            _cmdLine.usdSRGB = ftk::CmdLineValueOption<bool>::create(
                { "-usdSRGB" },
                "Enable sRGB color space.",
                "USD",
                true);
            _cmdLine.usdStageCache = ftk::CmdLineValueOption<size_t>::create(
                { "-usdStageCache" },
                "Stage cache size.",
                "USD",
                10);
            _cmdLine.usdDiskCache = ftk::CmdLineValueOption<size_t>::create(
                { "-usdDiskCache" },
                "Disk cache size in gigabytes. A size of zero disables the cache.",
                "USD",
                0);
#endif // TLRENDER_USD

            IApp::_init(
                context,
                argv,
                "tlbake",
                "Render a timeline to a movie or image sequence.",
                {
                    _cmdLine.input,
                    _cmdLine.output
                },
                {
                    _cmdLine.inOutRange,
                    _cmdLine.renderSize,
                    _cmdLine.outputPixelType,
                    _cmdLine.ocioFileName,
                    _cmdLine.ocioInput,
                    _cmdLine.ocioDisplay,
                    _cmdLine.ocioView,
                    _cmdLine.ocioLook,
                    _cmdLine.lutFileName,
                    _cmdLine.lutOrder,
                    _cmdLine.sequenceDefaultSpeed,
                    _cmdLine.sequenceThreadCount,
#if defined(TLRENDER_EXR)
                    _cmdLine.exrCompression,
                    _cmdLine.exrDWACompressionLevel,
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
                    _cmdLine.ffmpegCodec,
                    _cmdLine.ffmpegThreadCount,
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
                    _cmdLine.usdRenderWidth,
                    _cmdLine.usdComplexity,
                    _cmdLine.usdDrawMode,
                    _cmdLine.usdEnableLighting,
                    _cmdLine.usdSRGB,
                    _cmdLine.usdStageCache,
                    _cmdLine.usdDiskCache,
#endif // TLRENDER_USD
                });
        }

        App::App()
        {}

        App::~App()
        {}

        std::shared_ptr<App> App::create(
            const std::shared_ptr<ftk::Context>& context,
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
            _window = ftk::gl::Window::create(
                _context,
                "tlbake",
                ftk::Size2I(1, 1),
                static_cast<int>(ftk::gl::WindowOptions::MakeCurrent));

            // Read the timeline.
            timeline::Options options;
            options.ioOptions = _getIOOptions();
            _timeline = timeline::Timeline::create(
                _context,
                _cmdLine.input->getValue(),
                options);
            _timeRange = _timeline->getTimeRange();
            _print(ftk::Format("Timeline range: {0}-{1}").
                arg(_timeRange.start_time().value()).
                arg(_timeRange.end_time_inclusive().value()));
            _print(ftk::Format("Timeline speed: {0}").arg(_timeRange.duration().rate()));

            // Time range.
            if (_cmdLine.inOutRange->hasValue())
            {
                _timeRange = _cmdLine.inOutRange->getValue();
            }
            _print(ftk::Format("In/out range: {0}-{1}").
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
            _renderSize = ftk::Size2I(info.video[0].size.w, info.video[0].size.h);
            if (_cmdLine.renderSize->hasValue())
            {
                _renderSize = _cmdLine.renderSize->getValue();
            }
            _print(ftk::Format("Render size: {0}").arg(_renderSize));

            // Create the renderer.
            _render = timeline_gl::Render::create(_context->getLogSystem());
            ftk::gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.color = ftk::gl::offscreenColorDefault;
            _buffer = ftk::gl::OffscreenBuffer::create(_renderSize, offscreenBufferOptions);
            _bufferFlip = ftk::gl::OffscreenBuffer::create(_renderSize, offscreenBufferOptions);

            // Create the writer.
            const std::string output = _cmdLine.output->getValue();
            _writerPlugin = _context->getSystem<io::WriteSystem>()->getPlugin(file::Path(output));
            if (!_writerPlugin)
            {
                throw std::runtime_error(ftk::Format("Cannot open: \"{0}\"").arg(output));
            }
            _outputInfo.size.w = _renderSize.w;
            _outputInfo.size.h = _renderSize.h;
            _outputInfo.type = info.video[0].type;
            if (_cmdLine.outputPixelType->hasValue())
            {
                _outputInfo.type = _cmdLine.outputPixelType->getValue();
            }
            _outputInfo = _writerPlugin->getInfo(_outputInfo);
            if (ftk::ImageType::None == _outputInfo.type)
            {
                _outputInfo.type = ftk::ImageType::RGB_U8;
            }
            _print(ftk::Format("Output info: {0} {1}").
                arg(_outputInfo.size).
                arg(_outputInfo.type));
            _outputImage = ftk::Image::create(_outputInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(_outputInfo);
            ioInfo.videoTime = _timeRange;
            _writer = _writerPlugin->write(file::Path(output), ioInfo, _getIOOptions());
            if (!_writer)
            {
                throw std::runtime_error(ftk::Format("Cannot open: \"{0}\"").arg(output));
            }

            // Set options.
            if (_cmdLine.ocioFileName->hasValue() ||
                _cmdLine.ocioInput->hasValue() ||
                _cmdLine.ocioDisplay->hasValue() ||
                _cmdLine.ocioView->hasValue() ||
                _cmdLine.ocioLook->hasValue())
            {
                _ocioOptions.enabled = true;
                if (_cmdLine.ocioFileName->hasValue())
                {
                    _ocioOptions.fileName = _cmdLine.ocioFileName->getValue();
                }
                if (_cmdLine.ocioInput->hasValue())
                {
                    _ocioOptions.input = _cmdLine.ocioInput->getValue();
                }
                if (_cmdLine.ocioDisplay->hasValue())
                {
                    _ocioOptions.display = _cmdLine.ocioDisplay->getValue();
                }
                if (_cmdLine.ocioView->hasValue())
                {
                    _ocioOptions.view = _cmdLine.ocioView->getValue();
                }
                if (_cmdLine.ocioLook->hasValue())
                {
                    _ocioOptions.look = _cmdLine.ocioLook->getValue();
                }
            }
            if (_cmdLine.lutFileName->hasValue() ||
                _cmdLine.lutOrder->hasValue())
            {
                _lutOptions.enabled = true;
                if (_cmdLine.lutFileName->hasValue())
                {
                    _lutOptions.fileName = _cmdLine.lutFileName->getValue();
                }
                if (_cmdLine.lutOrder->hasValue())
                {
                    _lutOptions.order = _cmdLine.lutOrder->getValue();
                }
            }

            // Start the main loop.
            while (_running)
            {
                _tick();
            }

            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - _startTime;
            _print(ftk::Format("Seconds elapsed: {0}").arg(diff.count()));
            _print(ftk::Format("Average FPS: {0}").arg(_timeRange.duration().value() / diff.count()));
        }

        io::Options App::_getIOOptions() const
        {
            io::Options out;
            if (_cmdLine.sequenceDefaultSpeed->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.sequenceDefaultSpeed->getValue();
                out["SequenceIO/DefaultSpeed"] = ss.str();
            }
            if (_cmdLine.sequenceThreadCount->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.sequenceThreadCount->getValue();
                out["SequenceIO/ThreadCount"] = ss.str();
            }
#if defined(TLRENDER_EXR)
            if (_cmdLine.exrCompression->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.exrCompression->getValue();
                out["OpenEXR/Compression"] = ss.str();
            }
            if (_cmdLine.exrDWACompressionLevel->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.exrDWACompressionLevel->getValue();
                out["OpenEXR/DWACompressionLevel"] = ss.str();
            }
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
            if (_cmdLine.ffmpegCodec->hasValue())
            {
                out["FFmpeg/Codec"] = _cmdLine.ffmpegCodec->getValue();
            }
            if (_cmdLine.ffmpegThreadCount->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.ffmpegThreadCount->getValue();
                out["FFmpeg/ThreadCount"] = ss.str();
            }
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            if (_cmdLine.usdRenderWidth->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.usdRenderWidth->getValue();
                out["USD/RenderWidth"] = ss.str();
            }
            if (_cmdLine.usdComplexity->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.usdComplexity->getValue();
                out["USD/Complexity"] = ss.str();
            }
            if (_cmdLine.usdDrawMode->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.usdDrawMode->getValue();
                out["USD/DrawMode"] = ss.str();
            }
            if (_cmdLine.usdEnableLighting->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.usdEnableLighting->getValue();
                out["USD/EnableLighting"] = ss.str();
            }
            if (_cmdLine.usdSRGB->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.usdSRGB->getValue();
                out["USD/sRGB"] = ss.str();
            }
            if (_cmdLine.usdStageCache->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.usdStageCache->getValue();
                out["USD/StageCacheCount"] = ss.str();
            }
            if (_cmdLine.usdDiskCache->hasValue())
            {
                std::stringstream ss;
                ss << _cmdLine.usdDiskCache->getValue() * ftk::gigabyte;
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
            {
                ftk::gl::OffscreenBufferBinding binding(_buffer);
                _render->begin(_renderSize);
                _render->setOCIOOptions(_ocioOptions);
                _render->setLUTOptions(_lutOptions);
                const auto videoData = _timeline->getVideo(_inputTime).future.get();
                _render->drawVideo(
                    { videoData },
                    { ftk::Box2I(0, 0, _renderSize.w, _renderSize.h) });
                _render->end();
            }

            // Flip the image.
            ftk::gl::OffscreenBufferBinding binding(_bufferFlip);
            _render->begin(_renderSize);
            _render->setOCIOOptions(timeline::OCIOOptions());
            _render->setLUTOptions(timeline::LUTOptions());
            _render->drawTexture(
                _buffer->getColorID(),
                ftk::Box2I(0, 0, _renderSize.w, _renderSize.h));
            _render->end();

            // Write the frame.
            glPixelStorei(GL_PACK_ALIGNMENT, _outputInfo.layout.alignment);
#if defined(FTK_API_GL_4_1)
            glPixelStorei(GL_PACK_SWAP_BYTES, _outputInfo.layout.endian != ftk::getEndian());
#endif // FTK_API_GL_4_1
            const GLenum format = ftk::gl::getReadPixelsFormat(_outputInfo.type);
            const GLenum type = ftk::gl::getReadPixelsType(_outputInfo.type);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(ftk::Format("Cannot open: \"{0}\"").arg(_cmdLine.output->getValue()));
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
                _print(ftk::Format("Complete: {0}%").arg(static_cast<int>(c / static_cast<float>(d) * 100)));
            }
        }
    }
}
