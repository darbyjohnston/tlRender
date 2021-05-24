// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrGL/Util.h>

#include <tlrCore/Math.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tlr
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

    void App::_init(int argc, char* argv[])
    {
        IApp::_init(
            argc,
            argv,
            "tlrbake-glfw",
            "Convert an editorial timeline to a movie or image sequence.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "Input",
                    "The input timeline."),
                app::CmdLineValueArg<std::string>::create(
                    _output,
                    "Output",
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
                    _options.renderPixelType,
                    { "-renderPixelType", "-rp" },
                    string::Format("Render pixel type. Values: {0}").arg(string::join(imaging::getPixelTypeLabels(), ", "))),
                app::CmdLineValueOption<imaging::PixelType>::create(
                    _options.outputPixelType,
                    { "-outputPixelType", "-op" },
                    string::Format("Output pixel type. Values: {0}").arg(string::join(imaging::getPixelTypeLabels(), ", ")))
            });
    }

    App::App()
    {}

    App::~App()
    {
        _render.reset();
        _fontSystem.reset();
        if (_glfwWindow)
        {
            glfwDestroyWindow(_glfwWindow);
        }
        glfwTerminate();
    }

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
        
        // Read the timeline.
        _timeline = timeline::Timeline::create(_input);
        _duration = _timeline->getDuration();
        _print(string::Format("Timeline duration: {0}").arg(_duration.value()));
        _print(string::Format("Timeline speed: {0}").arg(_duration.rate()));

        // Time range.
        auto startTime = _options.startFrame >= 0 ?
            otime::RationalTime(_options.startFrame, _duration.rate()) :
            otime::RationalTime(0, _duration.rate());
        _range = _options.endFrame >= 0 ?
            otime::TimeRange::range_from_start_end_time_inclusive(startTime, otime::RationalTime(_options.endFrame, _duration.rate())) :
            otime::TimeRange::range_from_start_end_time(startTime, _duration);
        _currentTime = _range.start_time();
        _print(string::Format("Frame range: {0}-{1}").arg(_range.start_time().value()).arg(_range.end_time_inclusive().value()));

        // Render information.
        const auto& timelineInfo = _timeline->getImageInfo();
        _renderInfo.size = _options.renderSize.isValid() ?
            _options.renderSize :
            timelineInfo.size;
        _renderInfo.pixelType = _options.renderPixelType != imaging::PixelType::None ?
            _options.renderPixelType :
            timelineInfo.pixelType;
        _print(string::Format("Render info: {0}").arg(_renderInfo));

        // Output information.
        _outputInfo.size = _renderInfo.size;
        _outputInfo.pixelType = _options.outputPixelType != imaging::PixelType::None ?
            _options.outputPixelType :
            _renderInfo.pixelType;
        _print(string::Format("Output info: {0}").arg(_outputInfo));

        // Initialize GLFW.
        glfwSetErrorCallback(glfwErrorCallback);
        int glfwMajor = 0;
        int glfwMinor = 0;
        int glfwRevision = 0;
        glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
        {
            std::stringstream ss;
            ss << "GLFW version: " << glfwMajor << "." << glfwMinor << "." << glfwRevision;
            _printVerbose(ss.str());
        }
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
            "tlrbake-glfw",
            NULL,
            NULL);
        if (!_glfwWindow)
        {
            throw std::runtime_error("Cannot create window");
        }
        glfwMakeContextCurrent(_glfwWindow);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
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
        {
            std::stringstream ss;
            ss << "OpenGL version: " << glMajor << "." << glMinor << "." << glRevision;
            _printVerbose(ss.str());
        }

        // Create the renderer.
        _fontSystem = gl::FontSystem::create();
        _render = gl::Render::create();
        _buffer = gl::OffscreenBuffer::create(_renderInfo.size, _renderInfo.pixelType);

        // Create the I/O system.
        _ioSystem = io::System::create();
        io::VideoInfo videoInfo;
        videoInfo.info = _outputInfo;
        io::Info ioInfo;
        ioInfo.video.push_back(videoInfo);
        _writer = _ioSystem->write(_output, ioInfo);
        if (!_writer)
        {
            std::stringstream ss;
            ss << _output << ": Cannot open";
            throw std::runtime_error(ss.str());
        }

        // Start the main loop.
        while (_running)
        {
            _tick();
        }
    }

    void App::_tick()
    {
        // Tick the timeline.
        _timeline->tick();

        // Render the frame.
        {
            std::stringstream ss;
            ss << "Rendering frame: " << _currentTime.value();
            _print(ss.str());
        }
        gl::OffscreenBufferBinding binding(_buffer);
        _render->begin(_renderInfo.size);
        const auto& frame = _timeline->render(_currentTime + _timeline->getGlobalStartTime()).get();
        _render->drawImage(frame.image, math::BBox2f(0.F, 0.F, _renderInfo.size.w, _renderInfo.size.h));
        _render->end();

        // Write the frame.
        auto image = imaging::Image::create(_outputInfo);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        const GLenum format = gl::getReadPixelsFormat(_outputInfo.pixelType);
        const GLenum type = gl::getReadPixelsType(_outputInfo.pixelType);
        if (GL_NONE == format || GL_NONE == type)
        {
            std::stringstream ss;
            ss << _output << ": Cannot open";
            throw std::runtime_error(ss.str());
        }
        glReadPixels(
            0,
            0,
            _outputInfo.size.w,
            _outputInfo.size.h,
            format,
            type,
            image->getData());
        _writer->writeVideoFrame(_currentTime, image);

        // Advance the time.
        _currentTime += otime::RationalTime(1, _currentTime.rate());
        if (_currentTime > _range.end_time_inclusive())
        {
            _running = false;
        }
    }
}
