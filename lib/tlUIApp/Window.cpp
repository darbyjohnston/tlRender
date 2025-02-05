// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUIApp/Window.h>

#include <tlTimelineGL/Render.h>

#include <dtk/gl/GL.h>
#include <dtk/gl/OffscreenBuffer.h>
#include <dtk/gl/Util.h>
#include <dtk/gl/Window.h>
#if defined(dtk_API_GLES_2)
#include <dtk/gl/Mesh.h>
#include <dtk/gl/Shader.h>
#endif // dtk_API_GLES_2

#include <dtk/core/Format.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <codecvt>
#include <locale>

namespace tl
{
    namespace ui_app
    {
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

        struct Window::Private
        {
            std::shared_ptr<dtk::ObservableValue<dtk::Size2I> > windowSize;
            std::shared_ptr<dtk::ObservableValue<bool> > visible;
            std::shared_ptr<dtk::ObservableValue<bool> > fullScreen;
            std::shared_ptr<dtk::ObservableValue<bool> > floatOnTop;
            std::shared_ptr<dtk::ObservableValue<bool> > close;
            std::shared_ptr<dtk::ObservableValue<dtk::ImageType> > colorBuffer;

            std::shared_ptr<dtk::gl::Window> glWindow;
            dtk::Size2I frameBufferSize;
            float displayScale = 1.F;
            bool refresh = false;
            int modifiers = 0;
            std::shared_ptr<dtk::gl::TextureCache> textureCache;
            std::shared_ptr<timeline_gl::Render> render;
            std::shared_ptr<dtk::gl::OffscreenBuffer> offscreenBuffer;
#if defined(dtk_API_GLES_2)
            std::shared_ptr<gl::Shader> shader;
#endif // dtk_API_GLES_2
        };

        void Window::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& name,
            const std::shared_ptr<Window>& share)
        {
            IWindow::_init("tl::ui::Window", context, nullptr);
            TLRENDER_P();

            p.windowSize = dtk::ObservableValue<dtk::Size2I>::create(dtk::Size2I(1920, 1080));
            p.visible = dtk::ObservableValue<bool>::create(false);
            p.fullScreen = dtk::ObservableValue<bool>::create(false);
            p.floatOnTop = dtk::ObservableValue<bool>::create(false);
            p.close = dtk::ObservableValue<bool>::create(false);
            p.colorBuffer = dtk::ObservableValue<dtk::ImageType>::create(dtk::ImageType::RGBA_U8);

            p.glWindow = dtk::gl::Window::create(
                context,
                name,
                p.windowSize->get(),
                static_cast<int>(dtk::gl::WindowOptions::DoubleBuffer) |
                static_cast<int>(dtk::gl::WindowOptions::MakeCurrent),
                share ? share->getGLWindow() : nullptr);
            p.glWindow->setFrameBufferSizeCallback(
                [this](const dtk::Size2I& value)
                {
                    _p->frameBufferSize = value;
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                });
            p.glWindow->setContentScaleCallback(
                [this](const dtk::V2F& value)
                {
                    _p->displayScale = value.x;
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                });
            p.glWindow->setRefreshCallback(
                [this]
                {
                    _p->refresh = true;
                });
            p.glWindow->setCursorEnterCallback(
                [this](bool value)
                {
                    _cursorEnter(value);
                });
            p.glWindow->setCursorPosCallback(
                [this](const dtk::V2F& value)
                {
                    dtk::V2I pos;
#if defined(__APPLE__)
                    //! \bug The mouse position needs to be scaled on macOS?
                    pos.x = value.x * _p->displayScale;
                    pos.y = value.y * _p->displayScale;
#else // __APPLE__
                    pos.x = value.x;
                    pos.y = value.y;
#endif // __APPLE__
                    _cursorPos(pos);
                });
            p.glWindow->setButtonCallback(
                [this](int button, int action, int modifiers)
                {
                    _p->modifiers = modifiers;
                    _mouseButton(button, GLFW_PRESS == action, fromGLFWModifiers(modifiers));
                });
            p.glWindow->setScrollCallback(
                [this](const dtk::V2F& value)
                {
                    _scroll(value, fromGLFWModifiers(_p->modifiers));
                });
            p.glWindow->setKeyCallback(
                [this](int key, int scanCode, int action, int modifiers)
                {
                    TLRENDER_P();
                    p.modifiers = modifiers;
                    switch (action)
                    {
                    case GLFW_PRESS:
                    case GLFW_REPEAT:
                        _key(
                            fromGLFWKey(key),
                            true,
                            fromGLFWModifiers(modifiers));
                        break;
                    case GLFW_RELEASE:
                        _key(
                            fromGLFWKey(key),
                            false,
                            fromGLFWModifiers(modifiers));
                        break;
                    }
                });
            p.glWindow->setCharCallback(
                [this](unsigned int c)
                {
                    std::wstring_convert<std::codecvt_utf8<tl_char_t>, tl_char_t> utf32Convert;
                    _text(utf32Convert.to_bytes(c));
                });
            p.glWindow->setDropCallback(
                [this](int count, const char** fileNames)
                {
                    std::vector<std::string> tmp;
                    for (int i = 0; i < count; ++i)
                    {
                        tmp.push_back(fileNames[i]);
                    }
                    _drop(tmp);
                });

            p.frameBufferSize = p.glWindow->getFrameBufferSize();
            p.displayScale = p.glWindow->getContentScale().x;

            if (share)
            {
                p.textureCache = share->_p->render->getTextureCache();
            }
        }

        Window::Window() :
            _p(new Private)
        {}

        Window::~Window()
        {
            _makeCurrent();
        }

        std::shared_ptr<Window> Window::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& name,
            const std::shared_ptr<Window>& share)
        {
            auto out = std::shared_ptr<Window>(new Window);
            out->_init(context, name, share);
            return out;
        }

        const dtk::Size2I& Window::getWindowSize() const
        {
            return _p->windowSize->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::Size2I> > Window::observeWindowSize() const
        {
            return _p->windowSize;
        }

        void Window::setWindowSize(const dtk::Size2I& value)
        {
            _p->glWindow->setSize(value);
            setGeometry(dtk::Box2I(_geometry.x(), _geometry.y(), value.w, value.h));
        }

        std::shared_ptr<dtk::IObservableValue<bool> > Window::observeVisible() const
        {
            return _p->visible;
        }

        int Window::getScreen() const
        {
            return _p->glWindow->getScreen();
        }

        bool Window::isFullScreen() const
        {
            return _p->fullScreen->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > Window::observeFullScreen() const
        {
            return _p->fullScreen;
        }

        void Window::setFullScreen(bool value, int screen)
        {
            TLRENDER_P();
            p.glWindow->setFullScreen(value, screen);
            p.fullScreen->setIfChanged(value);
        }

        bool Window::isFloatOnTop() const
        {
            return _p->floatOnTop->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > Window::observeFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void Window::setFloatOnTop(bool value)
        {
            TLRENDER_P();
            p.glWindow->setFloatOnTop(value);
            p.floatOnTop->setIfChanged(value);
        }

        std::shared_ptr<dtk::IObservableValue<bool> > Window::observeClose() const
        {
            return _p->close;
        }

        dtk::ImageType Window::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > Window::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void Window::setColorBuffer(dtk::ImageType value)
        {
            if (_p->colorBuffer->setIfChanged(value))
            {
                _updates |= ui::Update::Draw;
            }
        }

        const std::shared_ptr<dtk::gl::Window>& Window::getGLWindow() const
        {
            return _p->glWindow;
        }

        void Window::setGeometry(const dtk::Box2I& value)
        {
            IWindow::setGeometry(value);
            for (const auto& i : _children)
            {
                i->setGeometry(value);
            }
            _p->windowSize->setIfChanged(value.size());
        }

        void Window::setVisible(bool value)
        {
            IWindow::setVisible(value);
            TLRENDER_P();
            if (p.visible->setIfChanged(value))
            {
                if (value)
                {
                    p.glWindow->show();
                }
                else
                {
                    p.glWindow->hide();
                }
            }
        }

        void Window::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWindow::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();

            if (_hasSizeUpdate(shared_from_this()))
            {
                ui::SizeHintEvent sizeHintEvent(
                    event.style,
                    event.iconLibrary,
                    event.fontSystem,
                    p.displayScale);
                _sizeHintEventRecursive(shared_from_this(), sizeHintEvent);

                setGeometry(dtk::Box2I(dtk::V2I(), p.frameBufferSize));

                _clipEventRecursive(
                    shared_from_this(),
                    _geometry,
                    !isVisible(false));
            }

            if (p.refresh || _hasDrawUpdate(shared_from_this()))
            {
                p.refresh = false;

                _makeCurrent();

                if (!p.render)
                {
                    p.render = timeline_gl::Render::create(
                        _context.lock(),
                        p.textureCache);
                }

                dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.color = p.colorBuffer->get();
                if (dtk::gl::doCreate(
                    p.offscreenBuffer,
                    p.frameBufferSize,
                    offscreenBufferOptions))
                {
                    p.offscreenBuffer = dtk::gl::OffscreenBuffer::create(
                        p.frameBufferSize,
                        offscreenBufferOptions);
                }
                if (p.offscreenBuffer)
                {
                    {
                        dtk::gl::OffscreenBufferBinding binding(p.offscreenBuffer);
                        dtk::RenderOptions renderOptions;
                        renderOptions.colorBuffer = p.colorBuffer->get();
                        p.render->begin(p.frameBufferSize, renderOptions);
                        ui::DrawEvent drawEvent(
                            event.style,
                            event.iconLibrary,
                            p.render,
                            event.fontSystem);
                        p.render->setClipRectEnabled(true);
                        _drawEventRecursive(
                            shared_from_this(),
                            dtk::Box2I(dtk::V2I(), p.frameBufferSize),
                            drawEvent);
                        p.render->setClipRectEnabled(false);
                        p.render->end();
                    }
                    glViewport(
                        0,
                        0,
                        GLsizei(p.frameBufferSize.w),
                        GLsizei(p.frameBufferSize.h));
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);
#if defined(dtk_API_GL_4_1)
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
#elif defined(dtk_API_GLES_2)
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
                            if (auto context = _context.lock())
                            {
                                context->log(
                                    "tl::ui_app::Window",
                                    dtk::Format("Cannot compile shader: {0}").arg(e.what()),
                                    log::Type::Error);
                            }
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
                                static_cast<float>(p.frameBufferSize.w),
                                0.F,
                                static_cast<float>(p.frameBufferSize.h),
                                -1.F,
                                1.F));
                        p.shader->setUniform("textureSampler", 0);

                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                        glBindTexture(GL_TEXTURE_2D, p.offscreenBuffer->getColorID());

                        auto mesh = geom::box(dtk::Box2I(
                            0,
                            0,
                            p.frameBufferSize.w,
                            p.frameBufferSize.h));
                        auto vboData = gl::convert(
                            mesh,
                            gl::VBOType::Pos2_F32_UV_U16,
                            dtk::RangeSizeT(0, mesh.triangles.size() - 1));
                        auto vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
                        vbo->copy(vboData);
                        auto vao = gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, vbo->getID());
                        vao->bind();
                        vao->draw(GL_TRIANGLES, 0, mesh.triangles.size() * 3);
                    }
#endif // dtk_API_GL_4_1

                    p.glWindow->swap();
                }

                _doneCurrent();
            }

            if (p.glWindow->shouldClose())
            {
                hide();
                p.close->setAlways(true);
            }
        }

        void Window::_makeCurrent()
        {
            TLRENDER_P();
            if (p.glWindow)
            {
                p.glWindow->makeCurrent();
            }
        }

        void Window::_doneCurrent()
        {
            TLRENDER_P();
            if (p.glWindow)
            {
                p.glWindow->doneCurrent();
            }
        }

        bool Window::_hasSizeUpdate(const std::shared_ptr<IWidget>& widget) const
        {
            bool out = widget->getUpdates() & ui::Update::Size;
            if (out)
            {
                //std::cout << "Size update: " << widget->getObjectName() << std::endl;
            }
            else
            {
                for (const auto& child : widget->getChildren())
                {
                    out |= _hasSizeUpdate(child);
                    if (out)
                    {
                        break;
                    }
                }
            }
            return out;
        }

        void Window::_sizeHintEventRecursive(
            const std::shared_ptr<IWidget>& widget,
            const ui::SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEventRecursive(child, event);
            }
            widget->sizeHintEvent(event);
        }

        bool Window::_hasDrawUpdate(const std::shared_ptr<IWidget>& widget) const
        {
            bool out = false;
            if (!widget->isClipped())
            {
                out = widget->getUpdates() & ui::Update::Draw;
                if (out)
                {
                    //std::cout << "Draw update: " << widget->getObjectName() << std::endl;
                }
                else
                {
                    for (const auto& child : widget->getChildren())
                    {
                        out |= _hasDrawUpdate(child);
                        if (out)
                        {
                            break;
                        }
                    }
                }
            }
            return out;
        }

        void Window::_drawEventRecursive(
            const std::shared_ptr<IWidget>& widget,
            const dtk::Box2I& drawRect,
            const ui::DrawEvent& event)
        {
            const dtk::Box2I& g = widget->getGeometry();
            if (!widget->isClipped() && g.w() > 0 && g.h() > 0)
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const dtk::Box2I childrenClipRect = dtk::intersect(
                    widget->getChildrenClipRect(),
                    drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const dtk::Box2I& childGeometry = child->getGeometry();
                    if (dtk::intersects(childGeometry, childrenClipRect))
                    {
                        _drawEventRecursive(
                            child,
                            dtk::intersect(childGeometry, childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }
    }
}
