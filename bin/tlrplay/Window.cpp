// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <sstream>

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

    void App::_createWindow()
    {
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

        GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
        _windowSize.w = std::min(static_cast<int>(_info.size.w * _options.windowScale), glfwVidmode->width);
        _windowSize.h = std::min(static_cast<int>(_info.size.h * _options.windowScale), glfwVidmode->height);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        _glfwWindow = glfwCreateWindow(
            _windowSize.w,
            _windowSize.h,
            "tlrplay",
            NULL,
            NULL);
        if (!_glfwWindow)
        {
            throw std::runtime_error("Cannot create window");
        }
        glfwSetWindowUserPointer(_glfwWindow, this);
        if (_options.fullScreen)
        {
            _fullscreenWindow();
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

        glfwSetKeyCallback(_glfwWindow, _keyCallback);

        glfwShowWindow(_glfwWindow);
    }

    void App::_destroyWindow()
    {
        if (_glfwWindow)
        {
            glfwDestroyWindow(_glfwWindow);
        }
        glfwTerminate();
    }

    void App::_fullscreenWindow()
    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(_glfwWindow, &width, &height);
        _windowSize.w = width;
        _windowSize.h = height;

        _options.fullScreen = true;

        GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
        glfwGetWindowPos(_glfwWindow, &_windowPos.x, &_windowPos.y);
        glfwSetWindowMonitor(_glfwWindow, glfwMonitor, 0, 0, glfwVidmode->width, glfwVidmode->height, glfwVidmode->refreshRate);
    }

    void App::_normalWindow()
    {
        _options.fullScreen = false;

        GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
        glfwSetWindowMonitor(_glfwWindow, NULL, _windowPos.x, _windowPos.y, _windowSize.w, _windowSize.h, 0);
    }

    void App::_fullscreenCallback(bool value)
    {
        if (value)
        {
            _fullscreenWindow();
        }
        else
        {
            _normalWindow();
        }
        {
            std::stringstream ss;
            ss << "Fullscreen: " << _options.fullScreen;
            _print(ss.str());
        }
    }

    void App::_keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
    {
        if (GLFW_RELEASE == action)
        {
            App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                app->exit();
                break;
            case GLFW_KEY_U:
                app->_fullscreenCallback(!app->_options.fullScreen);
                break;
            case GLFW_KEY_H:
                app->_hudCallback(!app->_options.hud);
                break;
            case GLFW_KEY_SPACE:
                app->_playbackCallback(Playback::Stop == app->_playback ? Playback::Forward : Playback::Stop);
                break;
            case GLFW_KEY_L:
                app->_loopPlaybackCallback(!app->_options.loopPlayback);
                break;
            case GLFW_KEY_HOME:
                app->_seek(otime::RationalTime(0, app->_duration.rate()));
                break;
            case GLFW_KEY_END:
                app->_seek(app->_duration - otime::RationalTime(1, app->_duration.rate()));
                break;
            case GLFW_KEY_LEFT:
                app->_seek(app->_currentTime - otime::RationalTime(1, app->_duration.rate()));
                break;
            case GLFW_KEY_RIGHT:
                app->_seek(app->_currentTime + otime::RationalTime(1, app->_duration.rate()));
                break;
            }
        }
    }

    void App::_shortcutsHelp()
    {
        std::cout <<
            "\n"
            "Keyboard shortcuts:\n"
            "\n"
            "    Escape - Exit\n"
            "    U      - Fullscreen mode\n"
            "    H      - HUD enabled\n"
            "    Space  - Start/stop playback\n"
            "    L      - Loop playback\n"
            "    Home   - Go to the start time\n"
            "    End    - Go to the end time\n"
            "    Left   - Go to the previous frame\n"
            "    Right  - Go to the next frame\n"
            "\n";
    }
}
