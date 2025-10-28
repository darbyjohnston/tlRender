// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/ContainerWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineGL/Render.h>

#include <ftk/UI/IWindow.h>
#include <ftk/UI/RowLayout.h>
#include <ftk/UI/IconSystem.h>
#include <ftk/GL/Init.h>
#include <ftk/GL/Mesh.h>
#include <ftk/GL/OffscreenBuffer.h>
#include <ftk/GL/Shader.h>
#include <ftk/Core/Context.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QGuiApplication>
#include <QMimeData>
#include <QTimer>

namespace tl
{
    namespace qtwidget
    {
        namespace
        {
            const size_t timeout = 5;

            class ContainerWindow : public ftk::IWindow
            {
                FTK_NON_COPYABLE(ContainerWindow);

            public:
                void _init(const std::shared_ptr<ftk::Context>& context)
                {
                    IWindow::_init(context, "tl::qtwidget::ContainerWindow", nullptr);
                }

                ContainerWindow()
                {}

            public:
                virtual ~ContainerWindow()
                {}

                static std::shared_ptr<ContainerWindow> create(
                    const std::shared_ptr<ftk::Context>& context)
                {
                    auto out = std::shared_ptr<ContainerWindow>(new ContainerWindow);
                    out->_init(context);
                    return out;
                }

                bool key(ftk::Key key, bool press, int modifiers)
                {
                    return _key(key, press, modifiers);
                }

                void text(const std::string& text)
                {
                    _text(text);
                }

                void cursorEnter(bool enter)
                {
                    _cursorEnter(enter);
                }

                void cursorPos(const ftk::V2I& value)
                {
                    _cursorPos(value);
                }

                void mouseButton(int button, bool press, int modifiers)
                {
                    _mouseButton(button, press, modifiers);
                }

                void scroll(const ftk::V2F& value, int modifiers)
                {
                    _scroll(value, modifiers);
                }

                void setGeometry(const ftk::Box2I& value) override
                {
                    IWindow::setGeometry(value);
                    for (const auto& i : getChildren())
                    {
                        i->setGeometry(value);
                    }
                }
            };
        }

        struct ContainerWidget::Private
        {
            std::weak_ptr<ftk::Context> context;
            std::shared_ptr<ftk::Style> style;
            std::shared_ptr<ftk::IconSystem> iconSystem;
            std::shared_ptr<ftk::FontSystem> fontSystem;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<ftk::IWidget> widget;
            std::shared_ptr<ContainerWindow> window;
            std::shared_ptr<ftk::gl::Shader> shader;
            std::shared_ptr<ftk::gl::OffscreenBuffer> buffer;
            std::shared_ptr<ftk::gl::VBO> vbo;
            std::shared_ptr<ftk::gl::VAO> vao;
            bool inputEnabled = true;
            std::chrono::steady_clock::time_point mouseWheelTimer;
            std::unique_ptr<QTimer> timer;
        };

        ContainerWidget::ContainerWidget(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::Style>& style,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            FTK_P();

            p.context = context;

            //QSurfaceFormat surfaceFormat;
            //surfaceFormat.setMajorVersion(4);
            //surfaceFormat.setMinorVersion(1);
            //surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            //surfaceFormat.setStencilBufferSize(8);
            //setFormat(surfaceFormat);

            p.style = style;
            p.iconSystem = context->getSystem<ftk::IconSystem>();
            p.fontSystem = context->getSystem<ftk::FontSystem>();
            p.window = ContainerWindow::create(context);

            _inputUpdate();
            _styleUpdate();

            p.timer.reset(new QTimer);
            p.timer->setTimerType(Qt::PreciseTimer);
            connect(p.timer.get(), &QTimer::timeout, this, &ContainerWidget::_timerUpdate);
            p.timer->start(timeout);
        }

        ContainerWidget::~ContainerWidget()
        {
            makeCurrent();
        }

        const std::shared_ptr<ftk::IWidget>& ContainerWidget::getWidget() const
        {
            return _p->widget;
        }

        void ContainerWidget::setWidget(const std::shared_ptr<ftk::IWidget>& widget)
        {
            FTK_P();
            if (p.widget)
            {
                p.widget->setParent(nullptr);
            }
            p.widget = widget;
            if (p.widget)
            {
                p.widget->setParent(p.window);
            }
        }

        bool ContainerWidget::isInputEnabled() const
        {
            return _p->inputEnabled;
        }

        void ContainerWidget::setInputEnabled(bool value)
        {
            FTK_P();
            if (value == p.inputEnabled)
                return;
            p.inputEnabled = value;
            _inputUpdate();
        }

        QSize ContainerWidget::minimumSizeHint() const
        {
            FTK_P();
            ftk::Size2I sizeHint;
            if (p.widget)
            {
                sizeHint = p.widget->getSizeHint();
            }
            const float devicePixelRatio = window()->devicePixelRatio();
            sizeHint.w /= devicePixelRatio;
            sizeHint.h /= devicePixelRatio;
            if (!sizeHint.isValid())
            {
                sizeHint.w = 1;
                sizeHint.h = 1;
            }
            return QSize(sizeHint.w, sizeHint.h);
        }

        QSize ContainerWidget::sizeHint() const
        {
            return minimumSizeHint();
        }

        void ContainerWidget::initializeGL()
        {
            FTK_P();
            initializeOpenGLFunctions();
            ftk::gl::initGLAD();
            if (auto context = p.context.lock())
            {
                try
                {
                    p.render = timeline_gl::Render::create(context->getLogSystem());

                    const std::string vertexSource =
                        "#version 410\n"
                        "\n"
                        "in vec3 vPos;\n"
                        "in vec2 vTexture;\n"
                        "out vec2 fTexture;\n"
                        "\n"
                        "uniform struct Transform\n"
                        "{\n"
                        "    mat4 mvp;\n"
                        "} transform;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    gl_Position = vec4(vPos, 1.0) * transform.mvp;\n"
                        "    fTexture = vTexture;\n"
                        "}\n";
                    const std::string fragmentSource =
                        "#version 410\n"
                        "\n"
                        "in vec2 fTexture;\n"
                        "out vec4 fColor;\n"
                        "\n"
                        "uniform sampler2D textureSampler;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    fColor = texture(textureSampler, fTexture);\n"
                        "}\n";
                    p.shader = ftk::gl::Shader::create(vertexSource, fragmentSource);
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::qtwidget::TimelineWidget",
                            e.what(),
                            ftk::LogType::Error);
                    }
                }
            }
            _sizeHintEvent();
        }

        void ContainerWidget::resizeGL(int w, int h)
        {
            FTK_P();
            _setGeometry();
            p.vao.reset();
            p.vbo.reset();
        }

        void ContainerWidget::paintGL()
        {
            FTK_P();
            const ftk::Size2I renderSize(_toUI(width()), _toUI(height()));
            if (_hasDrawUpdate(p.window))
            {
                try
                {
                    if (renderSize.isValid())
                    {
                        ftk::gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.color = ftk::ImageType::RGBA_U8;
                        if (ftk::gl::doCreate(p.buffer, renderSize, offscreenBufferOptions))
                        {
                            p.buffer = ftk::gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                        }
                    }
                    else
                    {
                        p.buffer.reset();
                    }

                    if (p.render && p.buffer)
                    {
                        ftk::gl::OffscreenBufferBinding binding(p.buffer);
                        ftk::RenderOptions renderOptions;
                        renderOptions.clearColor = p.style->getColorRole(ftk::ColorRole::Window);
                        p.render->begin(renderSize, renderOptions);
                        const float devicePixelRatio = window()->devicePixelRatio();
                        ftk::DrawEvent drawEvent(
                            p.fontSystem,
                            p.iconSystem,
                            devicePixelRatio,
                            p.style,
                            p.render);
                        p.render->setClipRectEnabled(true);
                        _drawEvent(p.window, ftk::Box2I(ftk::V2I(), renderSize), drawEvent);
                        p.render->setClipRectEnabled(false);
                        p.render->end();
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::qtwidget::ContainerWidget",
                            e.what(),
                            ftk::LogType::Error);
                    }
                }
            }

            glViewport(
                0,
                0,
                renderSize.w,
                renderSize.h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            if (p.buffer)
            {
                p.shader->bind();
                const auto pm = ftk::ortho(
                    0.F,
                    static_cast<float>(renderSize.w),
                    0.F,
                    static_cast<float>(renderSize.h),
                    -1.F,
                    1.F);
                p.shader->setUniform("transform.mvp", pm);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                const auto mesh = ftk::mesh(ftk::Box2I(0, 0, renderSize.w, renderSize.h));
                if (!p.vbo)
                {
                    p.vbo = ftk::gl::VBO::create(mesh.triangles.size() * 3, ftk::gl::VBOType::Pos2_F32_UV_U16);
                }
                if (p.vbo)
                {
                    p.vbo->copy(convert(mesh, ftk::gl::VBOType::Pos2_F32_UV_U16));
                }

                if (!p.vao && p.vbo)
                {
                    p.vao = ftk::gl::VAO::create(ftk::gl::VBOType::Pos2_F32_UV_U16, p.vbo->getID());
                }
                if (p.vao && p.vbo)
                {
                    p.vao->bind();
                    p.vao->draw(GL_TRIANGLES, 0, p.vbo->getSize());
                }
            }
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        void ContainerWidget::enterEvent(QEvent* event)
#else
        void ContainerWidget::enterEvent(QEnterEvent* event)
#endif // QT_VERSION
        {
            FTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorEnter(true);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                p.window->cursorPos(
                    ftk::V2I(_toUI(event->x()), _toUI(event->y())));
#endif // QT_VERSION
            }
        }

        void ContainerWidget::leaveEvent(QEvent* event)
        {
            FTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(ftk::V2I(-1, -1));
                p.window->cursorEnter(false);
            }
        }

        namespace
        {
            int fromQtModifiers(int value)
            {
                int out = 0;
                if (value & Qt::ShiftModifier)
                {
                    out |= static_cast<int>(ftk::KeyModifier::Shift);
                }
                if (value & Qt::ControlModifier)
                {
                    out |= static_cast<int>(ftk::KeyModifier::Control);
                }
                if (value & Qt::AltModifier)
                {
                    out |= static_cast<int>(ftk::KeyModifier::Alt);
                }
                return out;
            }
        }

        void ContainerWidget::mousePressEvent(QMouseEvent* event)
        {
            FTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(
                    ftk::V2I(_toUI(event->x()), _toUI(event->y())));
                int button = 0;
                if (event->button() == Qt::LeftButton)
                {
                    button = 1;
                }
                if (button != 0)
                {
                    p.window->mouseButton(
                        button,
                        true,
                        fromQtModifiers(event->modifiers()));
                }
            }
        }

        void ContainerWidget::mouseReleaseEvent(QMouseEvent* event)
        {
            FTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                int button = -1;
                if (event->button() == Qt::LeftButton)
                {
                    button = 0;
                }
                if (button != -1)
                {
                    p.window->mouseButton(
                        button,
                        false,
                        fromQtModifiers(event->modifiers()));
                }
            }
        }

        void ContainerWidget::mouseMoveEvent(QMouseEvent* event)
        {
            FTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(
                    ftk::V2I(_toUI(event->x()), _toUI(event->y())));
            }
        }

        void ContainerWidget::wheelEvent(QWheelEvent* event)
        {
            FTK_P();
            if (p.inputEnabled)
            {
                const auto now = std::chrono::steady_clock::now();
                const auto diff = std::chrono::duration<float>(now - p.mouseWheelTimer);
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                p.mouseWheelTimer = now;
                p.window->scroll(
                    ftk::V2F(
                        event->angleDelta().x() / 8.F / 15.F,
                        event->angleDelta().y() / 8.F / 15.F),
                    fromQtModifiers(event->modifiers()));
            }
        }

        namespace
        {
            ftk::Key fromQtKey(int key)
            {
                ftk::Key out = ftk::Key::Unknown;
                switch (key)
                {
                case Qt::Key_Enter: out = ftk::Key::Return; break;
                case Qt::Key_Escape: out = ftk::Key::Escape; break;
                case Qt::Key_Backspace: out = ftk::Key::Backspace; break;
                case Qt::Key_Tab: out = ftk::Key::Tab; break;
                case Qt::Key_Space: out = ftk::Key::Space; break;
                case Qt::Key_Exclam: out = ftk::Key::Exclaim; break;
                case Qt::Key_QuoteDbl: out = ftk::Key::DoubleQuote; break;
                case Qt::Key_NumberSign: out = ftk::Key::Hash; break;
                case Qt::Key_Percent: out = ftk::Key::Percent; break;
                case Qt::Key_Dollar: out = ftk::Key::Dollar; break;
                case Qt::Key_Ampersand: out = ftk::Key::Ampersand; break;
                case Qt::Key_Apostrophe: out = ftk::Key::SingleQuote; break;
                case Qt::Key_ParenLeft: out = ftk::Key::LeftParen; break;
                case Qt::Key_ParenRight: out = ftk::Key::RightParen; break;
                case Qt::Key_Asterisk: out = ftk::Key::Asterisk; break;
                case Qt::Key_Plus: out = ftk::Key::Plus; break;
                case Qt::Key_Comma: out = ftk::Key::Comma; break;
                case Qt::Key_Minus: out = ftk::Key::Minus; break;
                case Qt::Key_Period: out = ftk::Key::Period; break;
                case Qt::Key_Slash: out = ftk::Key::Slash; break;
                case Qt::Key_0: out = ftk::Key::_0; break;
                case Qt::Key_1: out = ftk::Key::_1; break;
                case Qt::Key_2: out = ftk::Key::_2; break;
                case Qt::Key_3: out = ftk::Key::_3; break;
                case Qt::Key_4: out = ftk::Key::_4; break;
                case Qt::Key_5: out = ftk::Key::_5; break;
                case Qt::Key_6: out = ftk::Key::_6; break;
                case Qt::Key_7: out = ftk::Key::_7; break;
                case Qt::Key_8: out = ftk::Key::_8; break;
                case Qt::Key_9: out = ftk::Key::_9; break;
                case Qt::Key_Colon: out = ftk::Key::Colon; break;
                case Qt::Key_Semicolon: out = ftk::Key::Semicolon; break;
                case Qt::Key_Less: out = ftk::Key::Less; break;
                case Qt::Key_Equal: out = ftk::Key::Equals; break;
                case Qt::Key_Greater: out = ftk::Key::Greater; break;
                case Qt::Key_Question: out = ftk::Key::Question; break;
                case Qt::Key_At: out = ftk::Key::At; break;
                case Qt::Key_BracketLeft: out = ftk::Key::LeftBracket; break;
                case Qt::Key_Backslash: out = ftk::Key::Backslash; break;
                case Qt::Key_BracketRight: out = ftk::Key::RightBracket; break;
                case Qt::Key_AsciiCircum: out = ftk::Key::Caret; break;
                case Qt::Key_Underscore: out = ftk::Key::Underscore; break;
                case Qt::Key_QuoteLeft: out = ftk::Key::BackQuote; break;
                case Qt::Key_A: out = ftk::Key::A; break;
                case Qt::Key_B: out = ftk::Key::B; break;
                case Qt::Key_C: out = ftk::Key::C; break;
                case Qt::Key_D: out = ftk::Key::D; break;
                case Qt::Key_E: out = ftk::Key::E; break;
                case Qt::Key_F: out = ftk::Key::F; break;
                case Qt::Key_G: out = ftk::Key::G; break;
                case Qt::Key_H: out = ftk::Key::H; break;
                case Qt::Key_I: out = ftk::Key::I; break;
                case Qt::Key_J: out = ftk::Key::J; break;
                case Qt::Key_K: out = ftk::Key::K; break;
                case Qt::Key_L: out = ftk::Key::L; break;
                case Qt::Key_M: out = ftk::Key::M; break;
                case Qt::Key_N: out = ftk::Key::N; break;
                case Qt::Key_O: out = ftk::Key::O; break;
                case Qt::Key_P: out = ftk::Key::P; break;
                case Qt::Key_Q: out = ftk::Key::Q; break;
                case Qt::Key_R: out = ftk::Key::R; break;
                case Qt::Key_S: out = ftk::Key::S; break;
                case Qt::Key_T: out = ftk::Key::T; break;
                case Qt::Key_U: out = ftk::Key::U; break;
                case Qt::Key_V: out = ftk::Key::V; break;
                case Qt::Key_W: out = ftk::Key::W; break;
                case Qt::Key_X: out = ftk::Key::X; break;
                case Qt::Key_Y: out = ftk::Key::Y; break;
                case Qt::Key_Z: out = ftk::Key::Z; break;
                case Qt::Key_CapsLock: out = ftk::Key::CapsLock; break;
                case Qt::Key_F1: out = ftk::Key::F1; break;
                case Qt::Key_F2: out = ftk::Key::F2; break;
                case Qt::Key_F3: out = ftk::Key::F3; break;
                case Qt::Key_F4: out = ftk::Key::F4; break;
                case Qt::Key_F5: out = ftk::Key::F5; break;
                case Qt::Key_F6: out = ftk::Key::F6; break;
                case Qt::Key_F7: out = ftk::Key::F7; break;
                case Qt::Key_F8: out = ftk::Key::F8; break;
                case Qt::Key_F9: out = ftk::Key::F9; break;
                case Qt::Key_F10: out = ftk::Key::F10; break;
                case Qt::Key_F11: out = ftk::Key::F11; break;
                case Qt::Key_F12: out = ftk::Key::F12; break;
                case Qt::Key_Print: out = ftk::Key::PrintScreen; break;
                case Qt::Key_ScrollLock: out = ftk::Key::ScrollLock; break;
                case Qt::Key_Pause: out = ftk::Key::Pause; break;
                case Qt::Key_Insert: out = ftk::Key::Insert; break;
                case Qt::Key_Home: out = ftk::Key::Home; break;
                case Qt::Key_PageUp: out = ftk::Key::PageUp; break;
                case Qt::Key_Delete: out = ftk::Key::Delete; break;
                case Qt::Key_End: out = ftk::Key::End; break;
                case Qt::Key_PageDown: out = ftk::Key::PageDown; break;
                case Qt::Key_Right: out = ftk::Key::Right; break;
                case Qt::Key_Left: out = ftk::Key::Left; break;
                case Qt::Key_Down: out = ftk::Key::Down; break;
                case Qt::Key_Up: out = ftk::Key::Up; break;
                case Qt::Key_NumLock: out = ftk::Key::NumLock; break;
                }
                return out;
            }
        }

        void ContainerWidget::keyPressEvent(QKeyEvent* event)
        {
            FTK_P();
            if (p.inputEnabled &&
                p.window->key(
                    fromQtKey(event->key()),
                    true,
                    fromQtModifiers(event->modifiers())))
            {
                event->accept();
            }
            else
            {
                QOpenGLWidget::keyPressEvent(event);
            }
        }

        void ContainerWidget::keyReleaseEvent(QKeyEvent* event)
        {
            FTK_P();
            if (p.inputEnabled &&
                p.window->key(
                    fromQtKey(event->key()),
                    false,
                    fromQtModifiers(event->modifiers())))
            {
                event->accept();
            }
            else
            {
                QOpenGLWidget::keyReleaseEvent(event);
            }
        }

        bool ContainerWidget::event(QEvent* event)
        {
            FTK_P();
            bool out = QOpenGLWidget::event(event);
            if (event->type() == QEvent::StyleChange)
            {
                _styleUpdate();
            }
            return out;
        }

        int ContainerWidget::_toUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        ftk::V2I ContainerWidget::_toUI(const ftk::V2I& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        int ContainerWidget::_fromUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : 0.F;
        }

        ftk::V2I ContainerWidget::_fromUI(const ftk::V2I& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : ftk::V2I();
        }

        void ContainerWidget::_tickEvent()
        {
            FTK_P();
            ftk::TickEvent tickEvent;
            _tickEvent(_p->window, true, true, tickEvent);
        }

        void ContainerWidget::_tickEvent(
            const std::shared_ptr<ftk::IWidget>& widget,
            bool visible,
            bool enabled,
            const ftk::TickEvent& event)
        {
            FTK_P();
            const bool parentsVisible = visible && widget->isVisible(false);
            const bool parentsEnabled = enabled && widget->isEnabled(false);
            for (const auto& child : widget->getChildren())
            {
                _tickEvent(
                    child,
                    parentsVisible,
                    parentsEnabled,
                    event);
            }
            widget->tickEvent(visible, enabled, event);
        }

        bool ContainerWidget::_hasSizeUpdate(const std::shared_ptr<ftk::IWidget>& widget) const
        {
            bool out = widget->hasSizeUpdate();
            if (out)
            {
                //std::cout << "Size update: " << widget->getObjectName() << std::endl;
            }
            else
            {
                for (const auto& child : widget->getChildren())
                {
                    out |= _hasSizeUpdate(child);
                }
            }
            return out;
        }

        void ContainerWidget::_sizeHintEvent()
        {
            FTK_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            ftk::SizeHintEvent sizeHintEvent(
                p.fontSystem,
                p.iconSystem,
                devicePixelRatio,
                p.style);
            _sizeHintEvent(p.window, sizeHintEvent);
        }

        void ContainerWidget::_sizeHintEvent(
            const std::shared_ptr<ftk::IWidget>& widget,
            const ftk::SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEvent(child, event);
            }
            widget->sizeHintEvent(event);
        }

        void ContainerWidget::_setGeometry()
        {
            FTK_P();
            const ftk::Box2I geometry(0, 0, _toUI(width()), _toUI(height()));
            p.window->setGeometry(geometry);
        }

        void ContainerWidget::_clipEvent()
        {
            FTK_P();
            const ftk::Box2I geometry(0, 0, _toUI(width()), _toUI(height()));
            _clipEvent(p.window, geometry, false);
        }

        void ContainerWidget::_clipEvent(
            const std::shared_ptr<ftk::IWidget>& widget,
            const ftk::Box2I& clipRect,
            bool clipped)
        {
            const ftk::Box2I& g = widget->getGeometry();
            clipped |= !ftk::intersects(g, clipRect);
            clipped |= !widget->isVisible(false);
            const ftk::Box2I clipRect2 = ftk::intersect(g, clipRect);
            widget->clipEvent(clipRect2, clipped);
            const ftk::Box2I childrenClipRect =
                ftk::intersect(widget->getChildrenClipRect(), clipRect2);
            for (const auto& child : widget->getChildren())
            {
                const ftk::Box2I& childGeometry = child->getGeometry();
                _clipEvent(
                    child,
                    ftk::intersect(childGeometry, childrenClipRect),
                    clipped);
            }
        }

        bool ContainerWidget::_hasDrawUpdate(const std::shared_ptr<ftk::IWidget>& widget) const
        {
            bool out = false;
            if (!widget->isClipped())
            {
                out = widget->hasDrawUpdate();
                if (out)
                {
                    //std::cout << "Draw update: " << widget->getObjectName() << std::endl;
                }
                else
                {
                    for (const auto& child : widget->getChildren())
                    {
                        out |= _hasDrawUpdate(child);
                    }
                }
            }
            return out;
        }

        void ContainerWidget::_drawEvent(
            const std::shared_ptr<ftk::IWidget>& widget,
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            const ftk::Box2I& g = widget->getGeometry();
            if (!widget->isClipped() && g.w() > 0 && g.h() > 0)
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const ftk::Box2I childrenClipRect =
                    ftk::intersect(widget->getChildrenClipRect(), drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const ftk::Box2I& childGeometry = child->getGeometry();
                    if (ftk::intersects(childGeometry, childrenClipRect))
                    {
                        _drawEvent(
                            child,
                            ftk::intersect(childGeometry, childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }

        void ContainerWidget::_inputUpdate()
        {
            FTK_P();
            setMouseTracking(p.inputEnabled);
            setFocusPolicy(p.inputEnabled ? Qt::StrongFocus : Qt::NoFocus);
            if (!p.inputEnabled)
            {
                p.window->cursorEnter(false);
            }
        }

        void ContainerWidget::_timerUpdate()
        {
            if (_p)
            {
                _tickEvent();
                if (_hasSizeUpdate(_p->window))
                {
                    _sizeHintEvent();
                    _setGeometry();
                    _clipEvent();
                    updateGeometry();
                }
                if (_hasDrawUpdate(_p->window))
                {
                    update();
                }
            }
        }

        void ContainerWidget::_styleUpdate()
        {
            /*FTK_P();
            const auto palette = this->palette();
            p.style->setColorRole(
                ftk::ColorRole::Window,
                fromQt(palette.color(QPalette::ColorRole::Window)));
            p.style->setColorRole(
                ftk::ColorRole::Base,
                fromQt(palette.color(QPalette::ColorRole::Base)));
            p.style->setColorRole(
                ftk::ColorRole::Button,
                fromQt(palette.color(QPalette::ColorRole::Button)));
            p.style->setColorRole(
                ftk::ColorRole::Text,
                fromQt(palette.color(QPalette::ColorRole::WindowText)));*/
        }
    }
}
