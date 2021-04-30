// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <sstream>

namespace tlr
{
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
    
    void App::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
    {
        App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
        app->_frameBufferSize.w = width;
        app->_frameBufferSize.h = height;
        app->_renderDirty = true;
    }

    void App::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
    {
        App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
        app->_contentScale.x = x;
        app->_contentScale.y = y;
        app->_renderDirty = true;
    }

    void App::_keyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int mods)
    {
        if (GLFW_RELEASE == action || GLFW_REPEAT == action)
        {
            App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
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
                app->_seekCallback(otime::RationalTime(0, app->_duration.rate()));
                break;
            case GLFW_KEY_END:
                app->_seekCallback(app->_duration - otime::RationalTime(1, app->_duration.rate()));
                break;
            case GLFW_KEY_LEFT:
                app->_seekCallback(app->_currentTime - otime::RationalTime(1, app->_duration.rate()));
                break;
            case GLFW_KEY_RIGHT:
                app->_seekCallback(app->_currentTime + otime::RationalTime(1, app->_duration.rate()));
                break;
            }
        }
    }
    
    void App::_printShortcutsHelp()
    {
        _print(
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
            "    Right  - Go to the next frame\n");
    }
}
