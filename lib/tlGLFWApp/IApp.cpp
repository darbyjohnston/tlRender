// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLFWApp/IApp.h>

#include <tlUI/EventLoop.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>

#include <tlCore/StringFormat.h>

#include <tlGlad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tl
{
    namespace glfw
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

        struct IApp::Private
        {
            Options options;

            GLFWwindow* glfwWindow = nullptr;
            imaging::Size windowSize;
            math::Vector2i windowPos;
            bool fullscreen = false;
            imaging::Size frameBufferSize;
            math::Vector2f contentScale = math::Vector2f(1.F, 1.F);

            std::shared_ptr<imaging::FontSystem> fontSystem;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::EventLoop> eventLoop;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;

            bool running = true;
        };

        void IApp::_init(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<app::ICmdLineArg> >& args,
            const std::vector<std::shared_ptr<app::ICmdLineOption> >& options)
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<app::ICmdLineOption> > options2 = options;
            options2.push_back(
                app::CmdLineValueOption<imaging::Size>::create(
                    p.options.windowSize,
                    { "-windowSize", "-ws" },
                    "Window size.",
                    string::Format("{0}x{1}").arg(p.options.windowSize.w).arg(p.options.windowSize.h)));
            options2.push_back(
                app::CmdLineFlagOption::create(
                    p.options.fullscreen,
                    { "-fullscreen", "-fs" },
                    "Enable full screen mode."));
            app::IApp::_init(
                argc,
                argv,
                context,
                cmdLineName,
                cmdLineSummary,
                args,
                options2);
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
            p.glfwWindow = glfwCreateWindow(
                p.options.windowSize.w,
                p.options.windowSize.h,
                "ui-glfw",
                NULL,
                NULL);
            if (!p.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }
            glfwSetWindowUserPointer(p.glfwWindow, this);
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(p.glfwWindow, &width, &height);
            p.frameBufferSize.w = width;
            p.frameBufferSize.h = height;
            glfwGetWindowContentScale(p.glfwWindow, &p.contentScale.x, &p.contentScale.y);
            glfwMakeContextCurrent(p.glfwWindow);
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
            const int glMajor = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
            const int glMinor = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
            const int glRevision = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_REVISION);
            _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
            glfwSetFramebufferSizeCallback(p.glfwWindow, _frameBufferSizeCallback);
            glfwSetWindowContentScaleCallback(p.glfwWindow, _windowContentScaleCallback);
            _setFullscreenWindow(p.options.fullscreen);
            glfwSetCursorEnterCallback(p.glfwWindow, _cursorEnterCallback);
            glfwSetCursorPosCallback(p.glfwWindow, _cursorPosCallback);
            glfwSetMouseButtonCallback(p.glfwWindow, _mouseButtonCallback);
            glfwSetKeyCallback(p.glfwWindow, _keyCallback);
            glfwShowWindow(p.glfwWindow);

            // Initialize the user interface.
            p.style = ui::Style::create(_context);
            p.iconLibrary = ui::IconLibrary::create(_context);
            p.fontSystem = imaging::FontSystem::create(_context);
            p.eventLoop = ui::EventLoop::create(
                p.style,
                p.iconLibrary,
                p.fontSystem,
                _context);

            // Create the renderer.
            p.render = gl::Render::create(_context);
        }

        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {
            TLRENDER_P();
            p.eventLoop.reset();
            p.render.reset();
            if (p.glfwWindow)
            {
                glfwDestroyWindow(p.glfwWindow);
            }
            glfwTerminate();
        }

        void IApp::run()
        {
            TLRENDER_P();
            if (_exit != 0)
            {
                return;
            }

            // Start the main loop.
            while (p.running && !glfwWindowShouldClose(p.glfwWindow))
            {
                glfwPollEvents();
                _tick();
                time::sleep(std::chrono::milliseconds(5));
            }
        }

        void IApp::exit()
        {
            _p->running = false;
        }

        const std::shared_ptr<ui::EventLoop> IApp::getEventLoop() const
        {
            return _p->eventLoop;
        }

        void IApp::_setFullscreenWindow(bool value)
        {
            TLRENDER_P();
            if (value == p.fullscreen)
                return;
            p.fullscreen = value;
            if (p.fullscreen)
            {
                int width = 0;
                int height = 0;
                glfwGetWindowSize(p.glfwWindow, &width, &height);
                p.windowSize.w = width;
                p.windowSize.h = height;

                GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
                glfwGetWindowPos(p.glfwWindow, &p.windowPos.x, &p.windowPos.y);
                glfwSetWindowMonitor(
                    p.glfwWindow,
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
                    p.glfwWindow,
                    NULL,
                    p.windowPos.x,
                    p.windowPos.y,
                    p.windowSize.w,
                    p.windowSize.h,
                    0);
            }
        }

        void IApp::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->frameBufferSize.w = width;
            app->_p->frameBufferSize.h = height;
        }
            
        void IApp::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->contentScale.x = x;
            app->_p->contentScale.y = y;
        }

        void IApp::_cursorEnterCallback(GLFWwindow* glfwWindow, int value)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->eventLoop->cursorEnter(GLFW_TRUE == value);
        }

        void IApp::_cursorPosCallback(GLFWwindow* glfwWindow, double x, double y)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->eventLoop->cursorPos(math::Vector2i(x, y));
        }

        void IApp::_mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            int modifiers = static_cast<int>(ui::KeyModifier::None);
            if (mods & GLFW_MOD_SHIFT)
            {
                modifiers |= static_cast<int>(ui::KeyModifier::Shift);
            }
            if (mods & GLFW_MOD_CONTROL)
            {
                modifiers |= static_cast<int>(ui::KeyModifier::Control);
            }
            if (mods & GLFW_MOD_ALT)
            {
                modifiers |= static_cast<int>(ui::KeyModifier::Alt);
            }
            app->_p->eventLoop->mouseButton(button, GLFW_PRESS == action, modifiers);
        }

        namespace
        {
            ui::Key toKey(int key)
            {
                ui::Key out = ui::Key::Unknown;
                switch (key)
                {
                case GLFW_KEY_SPACE: out = ui::Key::Space; break;
                case GLFW_KEY_APOSTROPHE: out = ui::Key::Apostrophe; break;
                case GLFW_KEY_COMMA: out = ui::Key::Comma; break;
                case GLFW_KEY_MINUS: out = ui::Key::Minus; break;
                case GLFW_KEY_PERIOD: out = ui::Key::Period; break;
                case GLFW_KEY_SLASH: out = ui::Key::Slash; break;
                case GLFW_KEY_0: out = ui::Key::_0; break;
                case GLFW_KEY_1: out = ui::Key::_1; break;
                case GLFW_KEY_2: out = ui::Key::_2; break;
                case GLFW_KEY_3: out = ui::Key::_3; break;
                case GLFW_KEY_4: out = ui::Key::_4; break;
                case GLFW_KEY_5: out = ui::Key::_5; break;
                case GLFW_KEY_6: out = ui::Key::_6; break;
                case GLFW_KEY_7: out = ui::Key::_7; break;
                case GLFW_KEY_8: out = ui::Key::_8; break;
                case GLFW_KEY_9: out = ui::Key::_9; break;
                case GLFW_KEY_SEMICOLON: out = ui::Key::Semicolon; break;
                case GLFW_KEY_EQUAL: out = ui::Key::Equal; break;
                case GLFW_KEY_A: out = ui::Key::A; break;
                case GLFW_KEY_B: out = ui::Key::B; break;
                case GLFW_KEY_C: out = ui::Key::C; break;
                case GLFW_KEY_D: out = ui::Key::D; break;
                case GLFW_KEY_E: out = ui::Key::E; break;
                case GLFW_KEY_F: out = ui::Key::F; break;
                case GLFW_KEY_G: out = ui::Key::G; break;
                case GLFW_KEY_H: out = ui::Key::H; break;
                case GLFW_KEY_I: out = ui::Key::I; break;
                case GLFW_KEY_J: out = ui::Key::J; break;
                case GLFW_KEY_K: out = ui::Key::K; break;
                case GLFW_KEY_L: out = ui::Key::L; break;
                case GLFW_KEY_M: out = ui::Key::M; break;
                case GLFW_KEY_N: out = ui::Key::N; break;
                case GLFW_KEY_O: out = ui::Key::O; break;
                case GLFW_KEY_P: out = ui::Key::P; break;
                case GLFW_KEY_Q: out = ui::Key::Q; break;
                case GLFW_KEY_R: out = ui::Key::R; break;
                case GLFW_KEY_S: out = ui::Key::S; break;
                case GLFW_KEY_T: out = ui::Key::T; break;
                case GLFW_KEY_U: out = ui::Key::U; break;
                case GLFW_KEY_V: out = ui::Key::V; break;
                case GLFW_KEY_W: out = ui::Key::W; break;
                case GLFW_KEY_X: out = ui::Key::X; break;
                case GLFW_KEY_Y: out = ui::Key::Y; break;
                case GLFW_KEY_Z: out = ui::Key::Z; break;
                case GLFW_KEY_LEFT_BRACKET: out = ui::Key::LeftBracket; break;
                case GLFW_KEY_BACKSLASH: out = ui::Key::Backslash; break;
                case GLFW_KEY_RIGHT_BRACKET: out = ui::Key::RightBracket; break;
                case GLFW_KEY_GRAVE_ACCENT: out = ui::Key::GraveAccent; break;
                case GLFW_KEY_ESCAPE: out = ui::Key::Escape; break;
                case GLFW_KEY_ENTER: out = ui::Key::Enter; break;
                case GLFW_KEY_TAB: out = ui::Key::Tab; break;
                case GLFW_KEY_BACKSPACE: out = ui::Key::Backspace; break;
                case GLFW_KEY_INSERT: out = ui::Key::Insert; break;
                case GLFW_KEY_DELETE: out = ui::Key::Delete; break;
                case GLFW_KEY_RIGHT: out = ui::Key::Right; break;
                case GLFW_KEY_LEFT: out = ui::Key::Left; break;
                case GLFW_KEY_DOWN: out = ui::Key::Down; break;
                case GLFW_KEY_UP: out = ui::Key::Up; break;
                case GLFW_KEY_PAGE_UP: out = ui::Key::PageUp; break;
                case GLFW_KEY_PAGE_DOWN: out = ui::Key::PageDown; break;
                case GLFW_KEY_HOME: out = ui::Key::Home; break;
                case GLFW_KEY_END: out = ui::Key::End; break;
                case GLFW_KEY_CAPS_LOCK: out = ui::Key::CapsLock; break;
                case GLFW_KEY_SCROLL_LOCK: out = ui::Key::ScrollLock; break;
                case GLFW_KEY_NUM_LOCK: out = ui::Key::NumLock; break;
                case GLFW_KEY_PRINT_SCREEN: out = ui::Key::PrintScreen; break;
                case GLFW_KEY_PAUSE: out = ui::Key::Pause; break;
                case GLFW_KEY_F1: out = ui::Key::F1; break;
                case GLFW_KEY_F2: out = ui::Key::F2; break;
                case GLFW_KEY_F3: out = ui::Key::F3; break;
                case GLFW_KEY_F4: out = ui::Key::F4; break;
                case GLFW_KEY_F5: out = ui::Key::F5; break;
                case GLFW_KEY_F6: out = ui::Key::F6; break;
                case GLFW_KEY_F7: out = ui::Key::F7; break;
                case GLFW_KEY_F8: out = ui::Key::F8; break;
                case GLFW_KEY_F9: out = ui::Key::F9; break;
                case GLFW_KEY_F10: out = ui::Key::F10; break;
                case GLFW_KEY_F11: out = ui::Key::F11; break;
                case GLFW_KEY_F12: out = ui::Key::F12; break;
                case GLFW_KEY_LEFT_SHIFT: out = ui::Key::LeftShift; break;
                case GLFW_KEY_LEFT_CONTROL: out = ui::Key::LeftControl; break;
                case GLFW_KEY_LEFT_ALT: out = ui::Key::LeftAlt; break;
                case GLFW_KEY_LEFT_SUPER: out = ui::Key::LeftSuper; break;
                case GLFW_KEY_RIGHT_SHIFT: out = ui::Key::RightShift; break;
                case GLFW_KEY_RIGHT_CONTROL: out = ui::Key::RightControl; break;
                case GLFW_KEY_RIGHT_ALT: out = ui::Key::RightAlt; break;
                case GLFW_KEY_RIGHT_SUPER: out = ui::Key::RightSuper; break;
                }
                return out;
            }
        }

        void IApp::_keyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int mods)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            switch (action)
            {
            case GLFW_PRESS:
                app->_p->eventLoop->key(toKey(key), true);
                break;
            case GLFW_RELEASE:
            case GLFW_REPEAT:
                app->_p->eventLoop->key(toKey(key), false);
                break;
            }
        }

        void IApp::_tick()
        {
            TLRENDER_P();

            _context->tick();

            p.eventLoop->tick();
            p.eventLoop->setSize(p.frameBufferSize);
            p.eventLoop->setContentScale(p.contentScale.x);

            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
            if (gl::doCreate(p.offscreenBuffer, p.frameBufferSize, offscreenBufferOptions))
            {
                p.offscreenBuffer = gl::OffscreenBuffer::create(
                    p.frameBufferSize,
                    offscreenBufferOptions);
            }
            if (p.eventLoop->hasDrawUpdate() && p.offscreenBuffer)
            {
                {
                    gl::OffscreenBufferBinding binding(p.offscreenBuffer);
                    p.render->begin(p.frameBufferSize);
                    p.eventLoop->draw(p.render);
                    p.render->end();
                }
                glViewport(
                    0,
                    0,
                    GLsizei(p.frameBufferSize.w),
                    GLsizei(p.frameBufferSize.h));
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);
                glBindFramebuffer(
                    GL_READ_FRAMEBUFFER,
                    p.offscreenBuffer->getID());
                glBlitFramebuffer(
                    0,
                    0,
                    p.frameBufferSize.w,
                    p.frameBufferSize.h,
                    0,
                    0,
                    p.frameBufferSize.w,
                    p.frameBufferSize.h,
                    GL_COLOR_BUFFER_BIT,
                    GL_LINEAR);
                glfwSwapBuffers(p.glfwWindow);
            }
        }
    }
}
