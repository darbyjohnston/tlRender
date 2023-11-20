// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLApp/IApp.h>

#include <tlUI/EventLoop.h>
#include <tlUI/IClipboard.h>
#include <tlUI/Window.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <codecvt>
#include <locale>

namespace tl
{
    namespace gl
    {
        namespace
        {
            class Clipboard : public ui::IClipboard
            {
                TLRENDER_NON_COPYABLE(Clipboard);

            public:
                Clipboard()
                {}

            public:
                virtual ~Clipboard()
                {}

                static std::shared_ptr<Clipboard> create(
                    GLFWwindow* glfwWindow,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<Clipboard>(new Clipboard);
                    out->_init(context);
                    return out;
                }

                void setWindow(GLFWwindow* glfwWindow)
                {
                    _glfwWindow = glfwWindow;
                }

                std::string getText() const override
                {
                    return _glfwWindow ? glfwGetClipboardString(_glfwWindow) : std::string();
                }

                void setText(const std::string& value) override
                {
                    if (_glfwWindow)
                    {
                        glfwSetClipboardString(_glfwWindow, value.c_str());
                    }
                }

            private:
                GLFWwindow* _glfwWindow = nullptr;
            };
        }

        struct IApp::Private
        {
            struct WindowData
            {
                std::shared_ptr<gl::GLFWWindow> glfw;
                std::shared_ptr<timeline::IRender> render;
                std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
                std::shared_ptr<observer::ValueObserver<math::Size2i> > sizeObserver;
                std::shared_ptr<observer::ValueObserver<bool> > fullScreenObserver;
                std::shared_ptr<observer::ValueObserver<bool> > floatOnTopObserver;
            };
            std::map<std::shared_ptr<ui::Window>, WindowData> windows;
            std::shared_ptr<ui::Window> activeWindow;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;

            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<Clipboard> clipboard;
            int modifiers = 0;
            std::shared_ptr<ui::EventLoop> eventLoop;
            std::shared_ptr<gl::Shader> shader;
            bool refresh = false;
            bool running = true;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<ui::Window> > > windowsObserver;
        };

        void IApp::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<app::ICmdLineArg> >& cmdLineArgs,
            const std::vector<std::shared_ptr<app::ICmdLineOption> >& cmdLineOptions)
        {
            TLRENDER_P();
            if (const GLFWvidmode* monitorMode = glfwGetVideoMode(
                glfwGetPrimaryMonitor()))
            {
                _options.windowSize.w = monitorMode->width * .7F;
                _options.windowSize.h = monitorMode->height * .7F;
            }
            std::vector<std::shared_ptr<app::ICmdLineOption> > cmdLineOptions2 = cmdLineOptions;
            cmdLineOptions2.push_back(
                app::CmdLineValueOption<math::Size2i>::create(
                    _options.windowSize,
                    { "-windowSize", "-ws" },
                    "Window size.",
                    string::Format("{0}x{1}").arg(_options.windowSize.w).arg(_options.windowSize.h)));
            cmdLineOptions2.push_back(
                app::CmdLineFlagOption::create(
                    _options.fullscreen,
                    { "-fullscreen", "-fs" },
                    "Enable full screen mode."));
            app::IApp::_init(
                argv,
                context,
                cmdLineName,
                cmdLineSummary,
                cmdLineArgs,
                cmdLineOptions2);
            if (_exit != 0)
            {
                return;
            }

            // Initialize the user interface.
            p.style = ui::Style::create(_context);
            p.iconLibrary = ui::IconLibrary::create(_context);
            p.clipboard = Clipboard::create(nullptr, _context);
            p.eventLoop = ui::EventLoop::create(
                p.style,
                p.iconLibrary,
                p.clipboard,
                _context);

            // Create observers.
            p.windowsObserver = observer::ListObserver<std::shared_ptr<ui::Window> >::create(
                p.eventLoop->observeWindows(),
                [this](const std::vector<std::shared_ptr<ui::Window> >& windows)
                {
                    _windowsUpdate(windows);
                });
        }

        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {}

        int IApp::run()
        {
            TLRENDER_P();
            while (0 == _exit && p.running && !p.windows.empty())
            {
                glfwPollEvents();
                _windowsClose();
                _context->tick();
                _tick();
                p.eventLoop->tick();
                _windowsDraw();
                time::sleep(std::chrono::milliseconds(1));
            }
            return _exit;
        }

        void IApp::exit(int r)
        {
            _exit = r;
            _p->running = false;
        }

        const std::shared_ptr<ui::EventLoop> IApp::getEventLoop() const
        {
            return _p->eventLoop;
        }

        const std::shared_ptr<ui::Style> IApp::getStyle() const
        {
            return _p->style;
        }

        void IApp::_setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            if (value == p.colorConfigOptions)
                return;
            p.colorConfigOptions = value;
            p.refresh = true;
        }

        void IApp::_setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.refresh = true;
        }

        void IApp::_drop(const std::vector<std::string>&)
        {}

        void IApp::_tick()
        {}

        namespace
        {
            int fromGLFWModifiers(int value)
            {
                int out = 0;
                if (value & GLFW_MOD_SHIFT)
                {
                    out |= static_cast<int>(ui::KeyModifier::Shift);
                }
                if (value & GLFW_MOD_CONTROL)
                {
                    out |= static_cast<int>(ui::KeyModifier::Control);
                }
                if (value & GLFW_MOD_ALT)
                {
                    out |= static_cast<int>(ui::KeyModifier::Alt);
                }
                if (value & GLFW_MOD_SUPER)
                {
                    out |= static_cast<int>(ui::KeyModifier::Super);
                }
                return out;
            }
        }

        namespace
        {
            ui::Key fromGLFWKey(int key)
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

#if defined(_WINDOWS)
            //! \bug https://social.msdn.microsoft.com/Forums/vstudio/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
            typedef unsigned int tl_char_t;
#else // _WINDOWS
            typedef char32_t tl_char_t;
#endif // _WINDOWS
        }

        void IApp::_windowsUpdate(const std::vector<std::shared_ptr<ui::Window> >& windows)
        {
            TLRENDER_P();
            for (const auto& window : windows)
            {
                const auto i = p.windows.find(window);
                if (i == p.windows.end())
                {
                    auto glfwWindow = gl::GLFWWindow::create(
                        _getCmdLineName(),
                        _options.windowSize,
                        _context);

                    glfwWindow->setFrameBufferSizeCallback(
                        [this, window](const math::Size2i& value)
                        {
                            _p->eventLoop->setWindowResolution(window, value);
                        });
                    glfwWindow->setContentScaleCallback(
                        [this, window](const math::Vector2f& value)
                        {
                            _p->eventLoop->setWindowScale(window, value.x);
                        });
                    glfwWindow->setRefreshCallback(
                        [this]
                        {
                            _p->refresh = true;
                        });
                    glfwWindow->setCursorEnterCallback(
                        [this, window](bool value)
                        {
                            _setActiveWindow(value ? window : nullptr);
                        });
                    glfwWindow->setCursorPosCallback(
                        [this, window](const math::Vector2f& value)
                        {
                            _setActiveWindow(window);

                            math::Vector2i pos;
#if defined(__APPLE__)
                            //! \bug The mouse position needs to be scaled on macOS?
                            pos.x = value.x * _p->contentScale.x;
                            pos.y = value.y * _p->contentScale.y;
#else // __APPLE__
                            pos.x = value.x;
                            pos.y = value.y;
#endif // __APPLE__
                            _p->eventLoop->cursorPos(window, pos);
                        });
                    glfwWindow->setButtonCallback(
                        [this, window](int button, int action, int modifiers)
                        {
                            _p->modifiers = modifiers;
                            _p->eventLoop->mouseButton(window, button, GLFW_PRESS == action, fromGLFWModifiers(modifiers));
                        });
                    glfwWindow->setScrollCallback(
                        [this, window](const math::Vector2f& value)
                        {
                            _p->eventLoop->scroll(window, value, fromGLFWModifiers(_p->modifiers));
                        });
                    glfwWindow->setKeyCallback(
                        [this, window](int key, int scanCode, int action, int modifiers)
                        {
                            TLRENDER_P();
                            p.modifiers = modifiers;
                            switch (action)
                            {
                            case GLFW_PRESS:
                            case GLFW_REPEAT:
                                p.eventLoop->key(
                                    window,
                                    fromGLFWKey(key),
                                    true,
                                    fromGLFWModifiers(modifiers));
                                break;
                            case GLFW_RELEASE:
                                p.eventLoop->key(
                                    window,
                                    fromGLFWKey(key),
                                    false,
                                    fromGLFWModifiers(modifiers));
                                break;
                            }
                        });
                    glfwWindow->setCharCallback(
                        [this, window](unsigned int c)
                        {
                            std::wstring_convert<std::codecvt_utf8<tl_char_t>, tl_char_t> utf32Convert;
                            _p->eventLoop->text(window, utf32Convert.to_bytes(c));
                        });
                    glfwWindow->setDropCallback(
                        [this, window](int count, const char** fileNames)
                        {
                            std::vector<std::string> tmp;
                            for (int i = 0; i < count; ++i)
                            {
                                tmp.push_back(fileNames[i]);
                            }
                            _drop(tmp);
                        });

                    p.windows[window].glfw = glfwWindow;
                    p.windows[window].sizeObserver = observer::ValueObserver<math::Size2i>::create(
                        window->observeSize(),
                        [glfwWindow](const math::Size2i& value)
                        {
                            glfwWindow->setSize(value);
                        });
                    p.windows[window].fullScreenObserver = observer::ValueObserver<bool>::create(
                        window->observeFullScreen(),
                        [glfwWindow](bool value)
                        {
                            glfwWindow->setFullScreen(value);
                        });
                    p.windows[window].floatOnTopObserver = observer::ValueObserver<bool>::create(
                        window->observeFloatOnTop(),
                        [glfwWindow](bool value)
                        {
                            glfwWindow->setFloatOnTop(value);
                        });

                    math::Size2i windowResolution;
                    glfwGetFramebufferSize(
                        glfwWindow->getGLFW(),
                        &windowResolution.w,
                        &windowResolution.h);
                    math::Vector2f windowScale;
                    glfwGetWindowContentScale(
                        glfwWindow->getGLFW(),
                        &windowScale.x,
                        &windowScale.y);
                    p.eventLoop->setWindowResolution(window, windowResolution);
                    p.eventLoop->setWindowScale(window, windowScale.x);
                }
            }

            auto i = p.windows.begin();
            while (i != p.windows.end())
            {
                const auto j = std::find(windows.begin(), windows.end(), i->first);
                if (j == windows.end())
                {
                    i = p.windows.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        void IApp::_setActiveWindow(const std::shared_ptr<ui::Window>& window)
        {
            TLRENDER_P();
            if (window == p.activeWindow)
                return;

            if (p.activeWindow)
            {
                _p->eventLoop->cursorEnter(p.activeWindow, false);
            }

            p.activeWindow = window;

            if (p.activeWindow)
            {
                _p->eventLoop->cursorEnter(p.activeWindow, true);
            }

            const auto i = p.windows.find(window);
            p.clipboard->setWindow(
                i != p.windows.end() ?
                i->second.glfw->getGLFW() :
                nullptr);
        }

        void IApp::_windowsClose()
        {
            TLRENDER_P();
            auto i = p.windows.begin();
            while (i != p.windows.end())
            {
                if (i->second.glfw->shouldClose())
                {
                    auto window = i->first;
                    i = p.windows.erase(i);
                    p.eventLoop->removeWindow(window);
                }
                else
                {
                    ++i;
                }
            }
        }

        void IApp::_windowsDraw()
        {
            TLRENDER_P();
            for (auto& i : p.windows)
            {
                if (p.eventLoop->hasDrawUpdate(i.first) || p.refresh)
                {
                    i.second.glfw->makeCurrent();

                    if (!i.second.render)
                    {
                        i.second.render = timeline::GLRender::create(_context);
                    }

                    math::Size2i frameBufferSize;
                    glfwGetFramebufferSize(
                        i.second.glfw->getGLFW(),
                        &frameBufferSize.w,
                        &frameBufferSize.h);
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                    if (gl::doCreate(
                        i.second.offscreenBuffer,
                        frameBufferSize,
                        offscreenBufferOptions))
                    {
                        i.second.offscreenBuffer = gl::OffscreenBuffer::create(
                            frameBufferSize,
                            offscreenBufferOptions);
                    }
                    if (i.second.offscreenBuffer)
                    {
                        {
                            gl::OffscreenBufferBinding binding(i.second.offscreenBuffer);
                            i.second.render->begin(
                                frameBufferSize,
                                p.colorConfigOptions,
                                p.lutOptions);
                            p.eventLoop->draw(i.first, i.second.render);
                            i.second.render->end();
                        }
                        glViewport(
                            0,
                            0,
                            GLsizei(frameBufferSize.w),
                            GLsizei(frameBufferSize.h));
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);
#if defined(TLRENDER_API_GL_4_1)
                        glBindFramebuffer(
                            GL_READ_FRAMEBUFFER,
                            i.second.offscreenBuffer->getID());
                        glBlitFramebuffer(
                            0,
                            0,
                            frameBufferSize.w,
                            frameBufferSize.h,
                            0,
                            0,
                            frameBufferSize.w,
                            frameBufferSize.h,
                            GL_COLOR_BUFFER_BIT,
                            GL_LINEAR);
#elif defined(TLRENDER_API_GLES_2)
                        if (!p.shader)
                        {
                            try
                            {
                                const std::string vertexSource =
                                    "precision mediump float;\n"
                                    "\n"
                                    "attribute vec3 vPos;\n"
                                    "attribute vec2 vTexture;\n"
                                    "varying vec2 fTexture;\n"
                                    "\n"
                                    "struct Transform\n"
                                    "{\n"
                                    "    mat4 mvp;\n"
                                    "};\n"
                                    "\n"
                                    "uniform Transform transform;\n"
                                    "\n"
                                    "void main()\n"
                                    "{\n"
                                    "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                                    "    fTexture = vTexture;\n"
                                    "}\n";
                                const std::string fragmentSource =
                                    "precision mediump float;\n"
                                    "\n"
                                    "varying vec2 fTexture;\n"
                                    "\n"
                                    "uniform sampler2D textureSampler;\n"
                                    "\n"
                                    "void main()\n"
                                    "{\n"
                                    "    gl_FragColor = texture2D(textureSampler, fTexture);\n"
                                    "}\n";
                                p.shader = gl::Shader::create(vertexSource, fragmentSource);
                            }
                            catch (const std::exception& e)
                            {
                                _log(string::Format("Cannot compile shader: {0}").arg(e.what()),
                                    log::Type::Error);
                            }
                        }
                        if (p.shader)
                        {
                            glBindFramebuffer(GL_FRAMEBUFFER, 0);
                            glDisable(GL_BLEND);
                            glDisable(GL_SCISSOR_TEST);

                            p.shader->bind();
                            p.shader->setUniform(
                                "transform.mvp",
                                math::ortho(
                                    0.F,
                                    static_cast<float>(frameBufferSize.w),
                                    0.F,
                                    static_cast<float>(frameBufferSize.h),
                                    -1.F,
                                    1.F));
                            p.shader->setUniform("textureSampler", 0);

                            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                            glBindTexture(GL_TEXTURE_2D, i.second.offscreenBuffer->getColorID());

                            auto mesh = geom::box(math::Box2i(
                                0,
                                0,
                                frameBufferSize.w,
                                frameBufferSize.h));
                            auto vboData = gl::convert(
                                mesh,
                                gl::VBOType::Pos2_F32_UV_U16,
                                math::SizeTRange(0, mesh.triangles.size() - 1));
                            auto vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
                            vbo->copy(vboData);
                            auto vao = gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, vbo->getID());
                            vao->bind();
                            vao->draw(GL_TRIANGLES, 0, mesh.triangles.size() * 3);
                        }
#endif // TLRENDER_API_GL_4_1

                        i.second.glfw->swap();
                    }

                    i.second.glfw->doneCurrent();
                }
            }
            p.refresh = false;
        }
    }
}
