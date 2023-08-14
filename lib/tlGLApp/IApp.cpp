// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLApp/IApp.h>

#include <tlUI/EventLoop.h>
#include <tlUI/IClipboard.h>

#include <tlTimeline/GLRender.h>

#include <tlIO/IOSystem.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/OffscreenBuffer.h>
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
                void _init(
                    GLFWwindow* glfwWindow,
                    const std::shared_ptr<system::Context>& context)
                {
                    IClipboard::_init(context);
                    _glfwWindow = glfwWindow;
                }

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
                    out->_init(glfwWindow, context);
                    return out;
                }

                std::string getText() const override
                {
                    return glfwGetClipboardString(_glfwWindow);
                }

                void setText(const std::string& value) override
                {
                    glfwSetClipboardString(_glfwWindow, value.c_str());
                }

            private:
                GLFWwindow* _glfwWindow = nullptr;
            };

            class Cursor
            {
            public:
                Cursor(GLFWwindow*, GLFWcursor*);

                ~Cursor();

            private:
                GLFWcursor* _cursor = nullptr;
            };

            Cursor::Cursor(GLFWwindow* window, GLFWcursor* cursor) :
                _cursor(cursor)
            {
                glfwSetCursor(window, cursor);
            }

            Cursor::~Cursor()
            {
                glfwDestroyCursor(_cursor);
            }
        }

        struct IApp::Private
        {
            Options options;

            std::shared_ptr<gl::GLFWWindow> window;
            std::shared_ptr<observer::Value<bool> > fullscreen;
            std::shared_ptr<observer::Value<bool> > floatOnTop;
            math::Size2i frameBufferSize;
            math::Vector2f contentScale = math::Vector2f(1.F, 1.F);
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            bool refresh = false;
            std::unique_ptr<Cursor> cursor;

            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<Clipboard> clipboard;
            int modifiers = 0;
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
                app::CmdLineValueOption<math::Size2i>::create(
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

            // Create observers.
            p.fullscreen = observer::Value<bool>::create(false);
            p.floatOnTop = observer::Value<bool>::create(false);

            // Create the window.
            p.window = gl::GLFWWindow::create(
                cmdLineName,
                p.options.windowSize,
                _context);
            p.frameBufferSize = p.window->getFrameBufferSize();
            p.contentScale = p.window->getContentScale();
            p.window->setFrameBufferSizeCallback(
                [this](const math::Size2i& value)
                {
                    _p->frameBufferSize = value;
                });
            p.window->setContentScaleCallback(
                [this](const math::Vector2f& value)
                {
                    _p->contentScale = value;
                });
            p.window->setRefreshCallback(
                [this]
                {
                    _p->refresh = true;
                });
            p.window->setCursorEnterCallback(
                [this](bool value)
                {
                    _p->eventLoop->cursorEnter(value);
                });
            p.window->setCursorPosCallback(
                [this](const math::Vector2f& value)
                {
                    math::Vector2i pos;
#if defined(__APPLE__)
                    //! \bug The mouse position needs to be scaled on macOS?
                    pos.x = value.x * _p->contentScale.x;
                    pos.y = value.y * _p->contentScale.y;
#else // __APPLE__
                    pos.x = value.x;
                    pos.y = value.y;
#endif // __APPLE__
                    _p->eventLoop->cursorPos(pos);
                });
            p.window->setButtonCallback(
                [this](int button, int action, int mods)
                {
                    _buttonCallback(button, action, mods);
                });
            p.window->setScrollCallback(
                [this](const math::Vector2f& value)
                {
                    _scrollCallback(value);
                });
            p.window->setKeyCallback(
                [this](int key, int scanCode, int action, int mods)
                {
                    _keyCallback(key, scanCode, action, mods);
                });
            p.window->setCharCallback(
                [this](unsigned int value)
                {
                    _charCallback(value);
                });
            p.window->setDropCallback(
                [this](int count, const char** fileNames)
                {
                    _dropCallback(count, fileNames);
                });
            setFullScreen(p.options.fullscreen);

            // Initialize the user interface.
            p.style = ui::Style::create(_context);
            p.iconLibrary = ui::IconLibrary::create(_context);
            p.clipboard = Clipboard::create(p.window->getGLFW(), _context);
            p.eventLoop = ui::EventLoop::create(
                p.style,
                p.iconLibrary,
                p.clipboard,
                _context);
            p.eventLoop->setCapture(
                [this](const math::Box2i& value)
                {
                    return _capture(value);
                });

            // Initialize the renderer.
            p.render = timeline::GLRender::create(_context);
        }

        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {}

        void IApp::run()
        {
            TLRENDER_P();
            if (_exit != 0)
            {
                return;
            }

            // Start the main loop.
            while (p.running && !p.window->shouldClose())
            {
                glfwPollEvents();

                _context->tick();

                _tick();

                p.eventLoop->setDisplaySize(p.frameBufferSize);
                p.eventLoop->setDisplayScale(p.contentScale.x);
                p.eventLoop->tick();

                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                if (gl::doCreate(p.offscreenBuffer, p.frameBufferSize, offscreenBufferOptions))
                {
                    p.offscreenBuffer = gl::OffscreenBuffer::create(
                        p.frameBufferSize,
                        offscreenBufferOptions);
                }
                if ((p.eventLoop->hasDrawUpdate() || p.refresh) &&
                    p.offscreenBuffer)
                {
                    p.refresh = false;
                    {
                        gl::OffscreenBufferBinding binding(p.offscreenBuffer);
                        p.render->begin(
                            p.frameBufferSize,
                            p.colorConfigOptions,
                            p.lutOptions);
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
                    p.window->swap();
                }

                time::sleep(std::chrono::milliseconds(5));
            }
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

        math::Size2i IApp::getWindowSize() const
        {
            return _p->window->getSize();
        }

        void IApp::setWindowSize(const math::Size2i& value)
        {
            _p->window->setSize(value);
        }

        bool IApp::isFullScreen() const
        {
            return _p->fullscreen->get();
        }

        std::shared_ptr<observer::IValue<bool> > IApp::observeFullScreen() const
        {
            return _p->fullscreen;
        }

        void IApp::setFullScreen(bool value)
        {
            TLRENDER_P();
            if (p.fullscreen->setIfChanged(value))
            {
                p.window->setFullScreen(value);
            }
        }

        bool IApp::isFloatOnTop() const
        {
            return _p->floatOnTop->get();
        }

        std::shared_ptr<observer::IValue<bool> > IApp::observeFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void IApp::setFloatOnTop(bool value)
        {
            TLRENDER_P();
            if (p.floatOnTop->setIfChanged(value))
            {
                p.window->setFloatOnTop(value);
            }
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

        std::shared_ptr<OffscreenBuffer> IApp::_capture(const math::Box2i& value)
        {
            TLRENDER_P();
            std::shared_ptr<OffscreenBuffer> out;
            try
            {
                OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                out = OffscreenBuffer::create(math::Size2i(value.w(), value.h()), offscreenBufferOptions);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, p.offscreenBuffer->getID());
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, out->getID());
                glBlitFramebuffer(
                    value.min.x,
                    p.frameBufferSize.h - 1 - value.min.y,
                    value.max.x,
                    p.frameBufferSize.h - 1 - value.max.y,
                    0,
                    0,
                    value.w(),
                    value.h(),
                    GL_COLOR_BUFFER_BIT,
                    GL_LINEAR);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
            catch (const std::exception&)
            {}
            return out;
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

        void IApp::_buttonCallback(int button, int action, int modifiers)
        {
            TLRENDER_P();
            p.modifiers = modifiers;
            p.eventLoop->mouseButton(button, GLFW_PRESS == action, fromGLFWModifiers(modifiers));
        }

        void IApp::_scrollCallback(const math::Vector2f& value)
        {
            TLRENDER_P();
            p.eventLoop->scroll(value, fromGLFWModifiers(p.modifiers));
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
        }

        void IApp::_keyCallback(int key, int scanCode, int action, int modifiers)
        {
            TLRENDER_P();
            p.modifiers = modifiers;
            switch (action)
            {
            case GLFW_PRESS:
            case GLFW_REPEAT:
                p.eventLoop->key(
                    fromGLFWKey(key),
                    true,
                    fromGLFWModifiers(modifiers));
                break;
            case GLFW_RELEASE:
                p.eventLoop->key(
                    fromGLFWKey(key),
                    false,
                    fromGLFWModifiers(modifiers));
                break;
            }
        }

        namespace
        {
#if defined(_WINDOWS)
            //! \bug https://social.msdn.microsoft.com/Forums/vstudio/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
            typedef unsigned int tl_char_t;
#else // _WINDOWS
            typedef char32_t tl_char_t;
#endif // _WINDOWS
        }

        void IApp::_charCallback(unsigned int c)
        {
            std::wstring_convert<std::codecvt_utf8<tl_char_t>, tl_char_t> utf32Convert;
            _p->eventLoop->text(utf32Convert.to_bytes(c));
        }

        void IApp::_dropCallback(int count, const char** fileNames)
        {
            std::vector<std::string> tmp;
            for (int i = 0; i < count; ++i)
            {
                tmp.push_back(fileNames[i]);
            }
            _drop(tmp);
        }
    }
}
