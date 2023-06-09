// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlBakeApp/App.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/File.h>
#include <tlCore/Math.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGlad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tl
{
    namespace bake
    {
        namespace
        {
            /*void APIENTRY glDebugOutput(
                GLenum         source,
                GLenum         type,
                GLuint         id,
                GLenum         severity,
                GLsizei        length,
                const GLchar * message,
                const void *   userParam)
            {
                switch (severity)
                {
                case GL_DEBUG_SEVERITY_HIGH:
                case GL_DEBUG_SEVERITY_MEDIUM:
                case GL_DEBUG_SEVERITY_LOW:
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                default:
                    std::cerr << "DEBUG: " << message << std::endl;
                    break;
                }
            }*/
        }

        void App::_init(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context)
        {
            IApp::_init(
                argc,
                argv,
                context,
                "tlbake",
                "Render a timeline to a movie or image sequence.",
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
                    app::CmdLineValueOption<otime::TimeRange>::create(
                        _options.inOutRange,
                        { "-inOutRange" },
                        "Set the in/out points range."),
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
                        _options.colorConfigOptions.fileName,
                        { "-colorConfig", "-cc" },
                        "Color configuration file name (e.g., config.ocio)."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.input,
                        { "-colorInput", "-ci" },
                        "Input color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.display,
                        { "-colorDisplay", "-cd" },
                        "Display color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.view,
                        { "-colorView", "-cv" },
                        "View color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.lutOptions.fileName,
                        { "-lut" },
                        "LUT file name."),
                    app::CmdLineValueOption<timeline::LUTOrder>::create(
                        _options.lutOptions.order,
                        { "-lutOrder" },
                        "LUT operation order.",
                        string::Format("{0}").arg(_options.lutOptions.order),
                        string::join(timeline::getLUTOrderLabels(), ", ")),
                    app::CmdLineValueOption<float>::create(
                        _options.sequenceDefaultSpeed,
                        { "-sequenceDefaultSpeed" },
                        "Default speed for image sequences.",
                        string::Format("{0}").arg(_options.sequenceDefaultSpeed)),
                    app::CmdLineValueOption<int>::create(
                        _options.sequenceThreadCount,
                        { "-sequenceThreadCount" },
                        "Number of threads for image sequence I/O.",
                        string::Format("{0}").arg(_options.sequenceThreadCount)),
#if defined(TLRENDER_EXR)
                    app::CmdLineValueOption<exr::Compression>::create(
                        _options.exrCompression,
                        { "-exrCompression" },
                        "OpenEXR output compression.",
                        string::Format("{0}").arg(_options.exrCompression),
                        string::join(exr::getCompressionLabels(), ", ")),
                    app::CmdLineValueOption<float>::create(
                        _options.exrDWACompressionLevel,
                        { "-exrDWACompressionLevel" },
                        "OpenEXR DWA compression level.",
                        string::Format("{0}").arg(_options.exrDWACompressionLevel)),
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
                    app::CmdLineValueOption<std::string>::create(
                        _options.ffmpegWriteProfile,
                        { "-ffmpegProfile", "-ffp" },
                        "FFmpeg output profile.",
                        std::string(),
                        string::join(ffmpeg::getProfileLabels(), ", ")),
                    app::CmdLineValueOption<int>::create(
                        _options.ffmpegThreadCount,
                        { "-ffmpegThreadCount" },
                        "Number of threads for FFmpeg I/O.",
                        string::Format("{0}").arg(_options.ffmpegThreadCount)),
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
                    app::CmdLineValueOption<size_t>::create(
                        _options.usdRenderWidth,
                        { "-usdRenderWidth" },
                        "USD render width.",
                        string::Format("{0}").arg(_options.usdRenderWidth)),
                    app::CmdLineValueOption<float>::create(
                        _options.usdComplexity,
                        { "-usdComplexity" },
                        "USD render complexity setting.",
                        string::Format("{0}").arg(_options.usdComplexity)),
                    app::CmdLineValueOption<usd::DrawMode>::create(
                        _options.usdDrawMode,
                        { "-usdDrawMode" },
                        "USD render draw mode.",
                        string::Format("{0}").arg(_options.usdDrawMode),
                        string::join(usd::getDrawModeLabels(), ", ")),
                    app::CmdLineValueOption<bool>::create(
                        _options.usdEnableLighting,
                        { "-usdEnableLighting" },
                        "USD render enable lighting setting.",
                        string::Format("{0}").arg(_options.usdEnableLighting)),
                    app::CmdLineValueOption<size_t>::create(
                        _options.usdStageCache,
                        { "-usdStageCache" },
                        "USD stage cache size.",
                        string::Format("{0}").arg(_options.usdStageCache)),
                    app::CmdLineValueOption<size_t>::create(
                        _options.usdDiskCache,
                        { "-usdDiskCache" },
                        "USD disk cache size in gigabytes. A size of zero disables the cache.",
                        string::Format("{0}").arg(_options.usdDiskCache)),
#endif // TLRENDER_USD
                });

            // Set I/O options.
            io::Options ioOptions;
            {
                std::stringstream ss;
                ss << _options.sequenceDefaultSpeed;
                ioOptions["SequenceIO/DefaultSpeed"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.sequenceThreadCount;
                ioOptions["SequenceIO/ThreadCount"] = ss.str();
            }
#if defined(TLRENDER_EXR)
            {
                std::stringstream ss;
                ss << _options.exrCompression;
                ioOptions["exr/Compression"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.exrDWACompressionLevel;
                ioOptions["exr/DWACompressionLevel"] = ss.str();
            }
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
            if (!_options.ffmpegWriteProfile.empty())
            {
                ioOptions["ffmpeg/WriteProfile"] = _options.ffmpegWriteProfile;
            }
            {
                std::stringstream ss;
                ss << _options.ffmpegThreadCount;
                ioOptions["ffmpeg/ThreadCount"] = ss.str();
            }
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            {
                std::stringstream ss;
                ss << _options.usdRenderWidth;
                ioOptions["usd/renderWidth"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdComplexity;
                ioOptions["usd/complexity"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdDrawMode;
                ioOptions["usd/drawMode"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdEnableLighting;
                ioOptions["usd/enableLighting"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdStageCache;
                ioOptions["usd/stageCacheCount"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.usdDiskCache * memory::gigabyte;
                ioOptions["usd/diskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD
            auto ioSystem = context->getSystem<io::System>();
            ioSystem->setOptions(ioOptions);
        }

        App::App()
        {}

        App::~App()
        {
            _timeline.reset();
            _buffer.reset();
            _render.reset();
            if (_glfwWindow)
            {
                glfwDestroyWindow(_glfwWindow);
            }
        }

        std::shared_ptr<App> App::create(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(argc, argv, context);
            return out;
        }

        void App::run()
        {
            if (_exit != 0)
            {
                return;
            }

            _startTime = std::chrono::steady_clock::now();

            // Create the window.
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
            //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            _glfwWindow = glfwCreateWindow(1, 1, "tlbake", NULL, NULL);
            if (!_glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }
            glfwMakeContextCurrent(_glfwWindow);
            if (!gladLoaderLoadGL())
            {
                throw std::runtime_error("Cannot initialize GLAD");
            }
            /*GLint flags = 0;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & static_cast<GLint>(GL_CONTEXT_FLAG_DEBUG_BIT))
            {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(glDebugOutput, _context.get());
                glDebugMessageControl(
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    0,
                    nullptr,
                    GL_TRUE);
            }*/
            const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
            const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
            const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
            _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));

            // Read the timeline.
            _timeline = timeline::Timeline::create(_input, _context);
            _timeRange = _timeline->getTimeRange();
            _print(string::Format("Timeline range: {0}-{1}").
                arg(_timeRange.start_time().value()).
                arg(_timeRange.end_time_inclusive().value()));
            _print(string::Format("Timeline speed: {0}").arg(_timeRange.duration().rate()));

            // Time range.
            if (time::isValid(_options.inOutRange))
            {
                _timeRange = _options.inOutRange;
            }
            _print(string::Format("In/out range: {0}-{1}").
                arg(_timeRange.start_time().value()).
                arg(_timeRange.end_time_inclusive().value()));
            _inputTime = _timeRange.start_time();
            _outputTime = otime::RationalTime(0.0, _timeRange.duration().rate());

            // Render information.
            const auto& info = _timeline->getIOInfo();
            if (info.video.empty())
            {
                throw std::runtime_error("No video information");
            }
            _renderSize = _options.renderSize.isValid() ?
                _options.renderSize :
                info.video[0].size;
            _print(string::Format("Render size: {0}").arg(_renderSize));

            // Create the renderer.
            _render = timeline::GLRender::create(_context);
            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
            _buffer = gl::OffscreenBuffer::create(_renderSize, offscreenBufferOptions);

            // Create the writer.
            _writerPlugin = _context->getSystem<io::System>()->getPlugin(file::Path(_output));
            if (!_writerPlugin)
            {
                throw std::runtime_error(string::Format("{0}: Cannot open").arg(_output));
            }
            io::Info ioInfo;
            _outputInfo.size = _renderSize;
            _outputInfo.pixelType = _options.outputPixelType != imaging::PixelType::None ?
                _options.outputPixelType :
                info.video[0].pixelType;
            _outputInfo = _writerPlugin->getWriteInfo(_outputInfo);
            if (imaging::PixelType::None == _outputInfo.pixelType)
            {
                _outputInfo.pixelType = imaging::PixelType::RGB_U8;
            }
            _print(string::Format("Output info: {0} {1}").
                arg(_outputInfo.size).
                arg(_outputInfo.pixelType));
            _outputImage = imaging::Image::create(_outputInfo);
            ioInfo.video.push_back(_outputInfo);
            ioInfo.videoTime = _timeRange;
            _writer = _writerPlugin->write(file::Path(_output), ioInfo);
            if (!_writer)
            {
                throw std::runtime_error(string::Format("{0}: Cannot open").arg(_output));
            }

            // Start the main loop.
            gl::OffscreenBufferBinding binding(_buffer);
            while (_running)
            {
                _tick();
            }

            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = now - _startTime;
            _print(string::Format("Seconds elapsed: {0}").arg(diff.count()));
            _print(string::Format("Average FPS: {0}").arg(_timeRange.duration().value() / diff.count()));
        }

        void App::_tick()
        {
            _context->tick();

            _printProgress();

            // Render the video.
            _render->begin(
                _renderSize,
                _options.colorConfigOptions,
                _options.lutOptions);
            const auto videoData = _timeline->getVideo(_inputTime).get();
            _render->drawVideo(
                { videoData },
                { math::BBox2i(0, 0, _renderSize.w, _renderSize.h) });
            _render->end();

            // Write the frame.
            glPixelStorei(GL_PACK_ALIGNMENT, _outputInfo.layout.alignment);
            glPixelStorei(GL_PACK_SWAP_BYTES, _outputInfo.layout.endian != memory::getEndian());
            const GLenum format = gl::getReadPixelsFormat(_outputInfo.pixelType);
            const GLenum type = gl::getReadPixelsType(_outputInfo.pixelType);
            if (GL_NONE == format || GL_NONE == type)
            {
                throw std::runtime_error(string::Format("{0}: Cannot open").arg(_output));
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
            _inputTime += otime::RationalTime(1, _inputTime.rate());
            if (_inputTime > _timeRange.end_time_inclusive())
            {
                _running = false;
            }
            _outputTime += otime::RationalTime(1, _outputTime.rate());
        }

        void App::_printProgress()
        {
            const int64_t c = static_cast<int64_t>(_inputTime.value() - _timeRange.start_time().value());
            const int64_t d = static_cast<int64_t>(_timeRange.duration().value());
            if (d >= 100 && c % (d / 100) == 0)
            {
                _print(string::Format("Complete: {0}%").arg(static_cast<int>(c / static_cast<float>(d) * 100)));
            }
        }
    }
}
