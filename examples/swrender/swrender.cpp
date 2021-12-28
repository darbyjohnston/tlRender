// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "swrender.h"

#include <tlrCore/Mesh.h>
#include <tlrCore/SoftwareRender.h>
#include <tlrCore/StringFormat.h>

#include <glad/gl.h>

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
    }

    void App::_init(int argc, char* argv[])
    {
        IApp::_init(
            argc,
            argv,
            "swrender",
            "Experimental software rendering.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "input",
                    "The input timeline.")
            });
    }

    App::App()
    {}

    App::~App()
    {
        _glRender.reset();
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
        auto timeline = timeline::Timeline::create(_input, _context);
        _timelinePlayer = timeline::TimelinePlayer::create(timeline, _context);

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
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        _glfwWindow = glfwCreateWindow(
            _windowSize.w,
            _windowSize.h,
            "swrender",
            NULL,
            NULL);
        if (!_glfwWindow)
        {
            throw std::runtime_error("Cannot create window");
        }
        glfwSetWindowUserPointer(_glfwWindow, this);
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(_glfwWindow, &width, &height);
        glfwGetWindowContentScale(_glfwWindow, &_contentScale.x, &_contentScale.y);
        glfwMakeContextCurrent(_glfwWindow);
        if (!gladLoaderLoadGL())
        {
            throw std::runtime_error("Cannot initialize GLAD");
        }
        const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
        const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
        const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
        _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
        glfwSetFramebufferSizeCallback(_glfwWindow, _frameBufferSizeCallback);
        glfwSetWindowContentScaleCallback(_glfwWindow, _windowContentScaleCallback);
        glfwShowWindow(_glfwWindow);
        glfwShowWindow(_glfwWindow);

        // Create the renderer.
        _render = render::SoftwareRender::create(_context);
        _glRender = gl::Render::create(_context);

        // Start the main loop.
        _timelinePlayer->setPlayback(timeline::Playback::Forward);
        while (_running && !glfwWindowShouldClose(_glfwWindow))
        {
            glfwPollEvents();
            _tick();
        }
    }

    void App::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
    {
        App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
        app->_windowSize = imaging::Size(width, height);
        app->_renderDirty = true;
    }

    void App::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
    {
        App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
        app->_contentScale.x = x;
        app->_contentScale.y = y;
        app->_renderDirty = true;
    }

    void App::_tick()
    {
        // Update.
        _timelinePlayer->tick();
        const auto& videoData = _timelinePlayer->observeVideo()->get();
        if (!timeline::isTimeEqual(videoData, _videoData))
        {
            _videoData = videoData;
            _renderDirty = true;
        }

        // Render the video.
        if (_renderDirty)
        {
            _render->begin(_windowSize);
            for (const auto& layer : _videoData.layers)
            {
                _render->drawImage(layer.image, math::BBox2f(0, 0, _windowSize.w, _windowSize.h));
            }
            _render->end();

            _glRender->begin(_windowSize);
            _glRender->drawImage(_render->getFrameBuffer(), math::BBox2f(0, 0, _windowSize.w, _windowSize.h));
            _glRender->end();
            glfwSwapBuffers(_glfwWindow);
            _renderDirty = false;

            _renderDirty = true;
        }
        else
        {
            time::sleep(std::chrono::microseconds(1000));
        }
    }
}

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto app = tlr::App::create(argc, argv);
        app->run();
        r = app->getExit();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
