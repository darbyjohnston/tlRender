// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppBake/App.h>

#include <tlGL/Render.h>
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
            void glfwErrorCallback(int, const char* description)
            {
                std::cerr << "GLFW ERROR: " << description << std::endl;
            }

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
                case GL_DEBUG_SEVERITY_HIGH_KHR:
                case GL_DEBUG_SEVERITY_MEDIUM_KHR:
                    std::cerr << "DEBUG: " << message << std::endl;
                    break;
                default: break;
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
                _options.colorConfig.fileName,
                { "-colorConfig", "-cc" },
                "Color configuration file name (e.g., config.ocio)."),
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
        {
            _buffer.reset();
            _render.reset();
            _fontSystem.reset();
            if (_glfwWindow)
            {
                glfwDestroyWindow(_glfwWindow);
            }
            glfwTerminate();
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
            const auto& info = _timeline->getIOInfo();
            if (info.video.empty())
            {
                throw std::runtime_error("No video information");
            }
            _renderSize = _options.renderSize.isValid() ?
                _options.renderSize :
                info.video[0].size;
            _print(string::Format("Render size: {0}").arg(_renderSize));

            // Initialize GLFW.
            glfwSetErrorCallback(glfwErrorCallback);
            int glfwMajor = 0;
            int glfwMinor = 0;
            int glfwRevision = 0;
            glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
            _log(string::Format("GLFW version: {0}.{1}.{2}").arg(glfwMajor).arg(glfwMinor).arg(glfwRevision));
            if (!glfwInit())
            {
                throw std::runtime_error("Cannot initialize GLFW");
            }

            // Create the window.
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
            //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            _glfwWindow = glfwCreateWindow(
                100,
                100,
                "tlbake",
                NULL,
                NULL);
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
                glDebugMessageCallback(glDebugOutput, context.get());
                glDebugMessageControl(
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    0,
                    nullptr,
                    GLFW_TRUE);
            }*/
            const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
            const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
            const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
            _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));

            // Create the renderer.
            _fontSystem = imaging::FontSystem::create();
            _render = gl::Render::create(_context);
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
            gl::OffscreenBufferBinding binding(_buffer);
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
            _render->drawVideo({ videoData });
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
            _writer->writeVideo(_currentTime, _outputImage);

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
}
