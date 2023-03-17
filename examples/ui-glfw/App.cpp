// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlUI/RowLayout.h>
#include <tlUI/TextLabel.h>

#include <tlGL/Render.h>

#include <tlCore/StringFormat.h>

#include <tlGlad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
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
                    "ui-glfw",
                    "Example GLFW user interface application.",
                    {},
                {
                    app::CmdLineValueOption<imaging::Size>::create(
                        _options.windowSize,
                        { "-windowSize", "-ws" },
                        "Window size.",
                        string::Format("{0}x{1}").arg(_options.windowSize.w).arg(_options.windowSize.h)),
                    app::CmdLineFlagOption::create(
                        _options.fullscreen,
                        { "-fullscreen", "-fs" },
                        "Enable full screen mode.")
                });
            }

            App::App()
            {}

            App::~App()
            {
                _render.reset();
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
                    _options.windowSize.w,
                    _options.windowSize.h,
                    "ui-glfw",
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
                _frameBufferSize.w = width;
                _frameBufferSize.h = height;
                glfwGetWindowContentScale(_glfwWindow, &_contentScale.x, &_contentScale.y);
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
                glfwSetFramebufferSizeCallback(_glfwWindow, _frameBufferSizeCallback);
                glfwSetWindowContentScaleCallback(_glfwWindow, _windowContentScaleCallback);
                _setFullscreenWindow(_options.fullscreen);
                glfwSetKeyCallback(_glfwWindow, _keyCallback);
                glfwShowWindow(_glfwWindow);

                // Create the renderer.
                _fontSystem = imaging::FontSystem::create(_context);
                _render = gl::Render::create(_context);

                // Print the shortcuts help.
                _printShortcutsHelp();

                // Initialize the user interface.
                _style = ui::Style::create(_context);
                auto textLabel = ui::TextLabel::create(_context);
                textLabel->setText("Hello world!");
                auto textLabel2 = ui::TextLabel::create(_context);
                textLabel2->setText("Goodbye world!");
                auto vLayout = ui::RowLayout::create(ui::Orientation::Vertical, _context);
                textLabel->setParent(vLayout);
                textLabel2->setParent(vLayout);
                _window = ui::Window::create(_context);
                vLayout->setParent(_window);
                _eventLoop = ui::EventLoop::create(_context);
                _context->addSystem(_eventLoop);
                _eventLoop->addWindow(_window);

                // Start the main loop.
                while (_running && !glfwWindowShouldClose(_glfwWindow))
                {
                    glfwPollEvents();
                    _tick();
                }
            }

            void App::exit()
            {
                _running = false;
            }

            void App::_setFullscreenWindow(bool value)
            {
                if (value == _fullscreen)
                    return;

                _fullscreen = value;

                if (_fullscreen)
                {
                    int width = 0;
                    int height = 0;
                    glfwGetWindowSize(_glfwWindow, &width, &height);
                    _windowSize.w = width;
                    _windowSize.h = height;

                    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
                    glfwGetWindowPos(_glfwWindow, &_windowPos.x, &_windowPos.y);
                    glfwSetWindowMonitor(
                        _glfwWindow,
                        glfwMonitor,
                        0,
                        0,
                        glfwVidmode->width,
                        glfwVidmode->height,
                        glfwVidmode->refreshRate);
                }
                else
                {
                    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                    glfwSetWindowMonitor(
                        _glfwWindow,
                        NULL,
                        _windowPos.x,
                        _windowPos.y,
                        _windowSize.w,
                        _windowSize.h,
                        0);
                }
            }

            void App::_fullscreenCallback(bool value)
            {
                _setFullscreenWindow(value);
                _log(string::Format("Fullscreen: {0}").arg(_fullscreen));
            }

            void App::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
            {
                App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                app->_frameBufferSize.w = width;
                app->_frameBufferSize.h = height;
            }

            void App::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
            {
                App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                app->_contentScale.x = x;
                app->_contentScale.y = y;
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
                        app->_fullscreenCallback(!app->_fullscreen);
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
                    "    U      - Fullscreen mode\n");
            }

            void App::_tick()
            {
                _context->tick();

                ui::SizeHintData sizeHintData;
                sizeHintData.style = _style;
                sizeHintData.contentScale = _contentScale.x;
                sizeHintData.fontSystem = _fontSystem;
                _window->sizeHint(sizeHintData);

                const math::BBox2i bbox(0, 0, _frameBufferSize.w, _frameBufferSize.h);
                _window->setGeometry(bbox);

                _render->begin(_frameBufferSize);
                ui::DrawData drawData;
                drawData.bbox = bbox;
                drawData.style = _style;
                drawData.contentScale = _contentScale.x;
                drawData.fontSystem = _fontSystem;
                drawData.render = _render;
                _window->draw(drawData);
                _render->end();

                glfwSwapBuffers(_glfwWindow);
            }
        }
    }
}
