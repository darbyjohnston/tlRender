// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/ContainerWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineGL/Render.h>

#include <feather-tk/ui/IClipboard.h>
#include <feather-tk/ui/IWindow.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/IconSystem.h>
#include <feather-tk/gl/Init.h>
#include <feather-tk/gl/Mesh.h>
#include <feather-tk/gl/OffscreenBuffer.h>
#include <feather-tk/gl/Shader.h>
#include <feather-tk/core/Context.h>

#include <QClipboard>
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

            class ContainerWindow : public feather_tk::IWindow
            {
                FEATHER_TK_NON_COPYABLE(ContainerWindow);

            public:
                void _init(const std::shared_ptr<feather_tk::Context>& context)
                {
                    IWindow::_init(context, "tl::qtwidget::ContainerWindow", nullptr);
                }

                ContainerWindow()
                {}

            public:
                virtual ~ContainerWindow()
                {}

                static std::shared_ptr<ContainerWindow> create(
                    const std::shared_ptr<feather_tk::Context>& context)
                {
                    auto out = std::shared_ptr<ContainerWindow>(new ContainerWindow);
                    out->_init(context);
                    return out;
                }

                bool key(feather_tk::Key key, bool press, int modifiers)
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

                void cursorPos(const feather_tk::V2I& value)
                {
                    _cursorPos(value);
                }

                void mouseButton(int button, bool press, int modifiers)
                {
                    _mouseButton(button, press, modifiers);
                }

                void scroll(const feather_tk::V2F& value, int modifiers)
                {
                    _scroll(value, modifiers);
                }

                void setGeometry(const feather_tk::Box2I& value) override
                {
                    IWindow::setGeometry(value);
                    for (const auto& i : getChildren())
                    {
                        i->setGeometry(value);
                    }
                }
            };

            class Clipboard : public feather_tk::IClipboard
            {
                FEATHER_TK_NON_COPYABLE(Clipboard);

            public:
                void _init(const std::shared_ptr<feather_tk::Context>& context)
                {
                    IClipboard::_init(context);
                }

                Clipboard()
                {}

            public:
                virtual ~Clipboard()
                {}

                static std::shared_ptr<Clipboard> create(
                    const std::shared_ptr<feather_tk::Context>& context)
                {
                    auto out = std::shared_ptr<Clipboard>(new Clipboard);
                    out->_init(context);
                    return out;
                }

                std::string getText() const override
                {
                    QClipboard* clipboard = QGuiApplication::clipboard();
                    return clipboard->text().toUtf8().data();
                }

                void setText(const std::string& value) override
                {
                    QClipboard* clipboard = QGuiApplication::clipboard();
                    clipboard->setText(QString::fromUtf8(value.c_str()));
                }
            };
        }

        struct ContainerWidget::Private
        {
            std::weak_ptr<feather_tk::Context> context;
            std::shared_ptr<feather_tk::Style> style;
            std::shared_ptr<feather_tk::IconSystem> iconSystem;
            std::shared_ptr<feather_tk::FontSystem> fontSystem;
            std::shared_ptr<Clipboard> clipboard;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<feather_tk::IWidget> widget;
            std::shared_ptr<ContainerWindow> window;
            std::shared_ptr<feather_tk::gl::Shader> shader;
            std::shared_ptr<feather_tk::gl::OffscreenBuffer> buffer;
            std::shared_ptr<feather_tk::gl::VBO> vbo;
            std::shared_ptr<feather_tk::gl::VAO> vao;
            bool inputEnabled = true;
            std::chrono::steady_clock::time_point mouseWheelTimer;
            std::unique_ptr<QTimer> timer;
        };

        ContainerWidget::ContainerWidget(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<feather_tk::Style>& style,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            FEATHER_TK_P();

            p.context = context;

            //QSurfaceFormat surfaceFormat;
            //surfaceFormat.setMajorVersion(4);
            //surfaceFormat.setMinorVersion(1);
            //surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            //surfaceFormat.setStencilBufferSize(8);
            //setFormat(surfaceFormat);

            p.style = style;
            p.iconSystem = context->getSystem<feather_tk::IconSystem>();
            p.fontSystem = context->getSystem<feather_tk::FontSystem>();
            p.clipboard = Clipboard::create(context);
            p.window = ContainerWindow::create(context);
            p.window->setClipboard(p.clipboard);

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

        const std::shared_ptr<feather_tk::IWidget>& ContainerWidget::getWidget() const
        {
            return _p->widget;
        }

        void ContainerWidget::setWidget(const std::shared_ptr<feather_tk::IWidget>& widget)
        {
            FEATHER_TK_P();
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
            FEATHER_TK_P();
            if (value == p.inputEnabled)
                return;
            p.inputEnabled = value;
            _inputUpdate();
        }

        QSize ContainerWidget::minimumSizeHint() const
        {
            FEATHER_TK_P();
            feather_tk::Size2I sizeHint;
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
            FEATHER_TK_P();
            initializeOpenGLFunctions();
            feather_tk::gl::initGLAD();
            if (auto context = p.context.lock())
            {
                try
                {
                    p.render = timeline_gl::Render::create(context);

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
                    p.shader = feather_tk::gl::Shader::create(vertexSource, fragmentSource);
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::qtwidget::TimelineWidget",
                            e.what(),
                            feather_tk::LogType::Error);
                    }
                }
            }
            _sizeHintEvent();
        }

        void ContainerWidget::resizeGL(int w, int h)
        {
            FEATHER_TK_P();
            _setGeometry();
            p.vao.reset();
            p.vbo.reset();
        }

        void ContainerWidget::paintGL()
        {
            FEATHER_TK_P();
            const feather_tk::Size2I renderSize(_toUI(width()), _toUI(height()));
            if (_hasDrawUpdate(p.window))
            {
                try
                {
                    if (renderSize.isValid())
                    {
                        feather_tk::gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.color = feather_tk::ImageType::RGBA_U8;
                        if (feather_tk::gl::doCreate(p.buffer, renderSize, offscreenBufferOptions))
                        {
                            p.buffer = feather_tk::gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                        }
                    }
                    else
                    {
                        p.buffer.reset();
                    }

                    if (p.render && p.buffer)
                    {
                        feather_tk::gl::OffscreenBufferBinding binding(p.buffer);
                        feather_tk::RenderOptions renderOptions;
                        renderOptions.clearColor = p.style->getColorRole(feather_tk::ColorRole::Window);
                        p.render->begin(renderSize, renderOptions);
                        const float devicePixelRatio = window()->devicePixelRatio();
                        feather_tk::DrawEvent drawEvent(
                            p.fontSystem,
                            p.iconSystem,
                            devicePixelRatio,
                            p.style,
                            p.render);
                        p.render->setClipRectEnabled(true);
                        _drawEvent(p.window, feather_tk::Box2I(feather_tk::V2I(), renderSize), drawEvent);
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
                            feather_tk::LogType::Error);
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
                const auto pm = feather_tk::ortho(
                    0.F,
                    static_cast<float>(renderSize.w),
                    0.F,
                    static_cast<float>(renderSize.h),
                    -1.F,
                    1.F);
                p.shader->setUniform("transform.mvp", pm);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                const auto mesh = feather_tk::mesh(feather_tk::Box2I(0, 0, renderSize.w, renderSize.h));
                if (!p.vbo)
                {
                    p.vbo = feather_tk::gl::VBO::create(mesh.triangles.size() * 3, feather_tk::gl::VBOType::Pos2_F32_UV_U16);
                }
                if (p.vbo)
                {
                    p.vbo->copy(convert(mesh, feather_tk::gl::VBOType::Pos2_F32_UV_U16));
                }

                if (!p.vao && p.vbo)
                {
                    p.vao = feather_tk::gl::VAO::create(feather_tk::gl::VBOType::Pos2_F32_UV_U16, p.vbo->getID());
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
            FEATHER_TK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorEnter(true);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                p.window->cursorPos(
                    feather_tk::V2I(_toUI(event->x()), _toUI(event->y())));
#endif // QT_VERSION
            }
        }

        void ContainerWidget::leaveEvent(QEvent* event)
        {
            FEATHER_TK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(feather_tk::V2I(-1, -1));
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
                    out |= static_cast<int>(feather_tk::KeyModifier::Shift);
                }
                if (value & Qt::ControlModifier)
                {
                    out |= static_cast<int>(feather_tk::KeyModifier::Control);
                }
                if (value & Qt::AltModifier)
                {
                    out |= static_cast<int>(feather_tk::KeyModifier::Alt);
                }
                return out;
            }
        }

        void ContainerWidget::mousePressEvent(QMouseEvent* event)
        {
            FEATHER_TK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(
                    feather_tk::V2I(_toUI(event->x()), _toUI(event->y())));
                int button = -1;
                if (event->button() == Qt::LeftButton)
                {
                    button = 0;
                }
                if (button != -1)
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
            FEATHER_TK_P();
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
            FEATHER_TK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(
                    feather_tk::V2I(_toUI(event->x()), _toUI(event->y())));
            }
        }

        void ContainerWidget::wheelEvent(QWheelEvent* event)
        {
            FEATHER_TK_P();
            if (p.inputEnabled)
            {
                const auto now = std::chrono::steady_clock::now();
                const auto diff = std::chrono::duration<float>(now - p.mouseWheelTimer);
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                p.mouseWheelTimer = now;
                p.window->scroll(
                    feather_tk::V2F(
                        event->angleDelta().x() / 8.F / 15.F,
                        event->angleDelta().y() / 8.F / 15.F),
                    fromQtModifiers(event->modifiers()));
            }
        }

        namespace
        {
            feather_tk::Key fromQtKey(int key)
            {
                feather_tk::Key out = feather_tk::Key::Unknown;
                switch (key)
                {
                case Qt::Key_Space: out = feather_tk::Key::Space; break;
                case Qt::Key_Apostrophe: out = feather_tk::Key::Apostrophe; break;
                case Qt::Key_Comma: out = feather_tk::Key::Comma; break;
                case Qt::Key_Minus: out = feather_tk::Key::Minus; break;
                case Qt::Key_Period: out = feather_tk::Key::Period; break;
                case Qt::Key_Slash: out = feather_tk::Key::Slash; break;
                case Qt::Key_0: out = feather_tk::Key::_0; break;
                case Qt::Key_1: out = feather_tk::Key::_1; break;
                case Qt::Key_2: out = feather_tk::Key::_2; break;
                case Qt::Key_3: out = feather_tk::Key::_3; break;
                case Qt::Key_4: out = feather_tk::Key::_4; break;
                case Qt::Key_5: out = feather_tk::Key::_5; break;
                case Qt::Key_6: out = feather_tk::Key::_6; break;
                case Qt::Key_7: out = feather_tk::Key::_7; break;
                case Qt::Key_8: out = feather_tk::Key::_8; break;
                case Qt::Key_9: out = feather_tk::Key::_9; break;
                case Qt::Key_Semicolon: out = feather_tk::Key::Semicolon; break;
                case Qt::Key_Equal: out = feather_tk::Key::Equal; break;
                case Qt::Key_A: out = feather_tk::Key::A; break;
                case Qt::Key_B: out = feather_tk::Key::B; break;
                case Qt::Key_C: out = feather_tk::Key::C; break;
                case Qt::Key_D: out = feather_tk::Key::D; break;
                case Qt::Key_E: out = feather_tk::Key::E; break;
                case Qt::Key_F: out = feather_tk::Key::F; break;
                case Qt::Key_G: out = feather_tk::Key::G; break;
                case Qt::Key_H: out = feather_tk::Key::H; break;
                case Qt::Key_I: out = feather_tk::Key::I; break;
                case Qt::Key_J: out = feather_tk::Key::J; break;
                case Qt::Key_K: out = feather_tk::Key::K; break;
                case Qt::Key_L: out = feather_tk::Key::L; break;
                case Qt::Key_M: out = feather_tk::Key::M; break;
                case Qt::Key_N: out = feather_tk::Key::N; break;
                case Qt::Key_O: out = feather_tk::Key::O; break;
                case Qt::Key_P: out = feather_tk::Key::P; break;
                case Qt::Key_Q: out = feather_tk::Key::Q; break;
                case Qt::Key_R: out = feather_tk::Key::R; break;
                case Qt::Key_S: out = feather_tk::Key::S; break;
                case Qt::Key_T: out = feather_tk::Key::T; break;
                case Qt::Key_U: out = feather_tk::Key::U; break;
                case Qt::Key_V: out = feather_tk::Key::V; break;
                case Qt::Key_W: out = feather_tk::Key::W; break;
                case Qt::Key_X: out = feather_tk::Key::X; break;
                case Qt::Key_Y: out = feather_tk::Key::Y; break;
                case Qt::Key_Z: out = feather_tk::Key::Z; break;
                case Qt::Key_BracketLeft: out = feather_tk::Key::LeftBracket; break;
                case Qt::Key_Backslash: out = feather_tk::Key::Backslash; break;
                case Qt::Key_BracketRight: out = feather_tk::Key::RightBracket; break;
                case Qt::Key_Agrave: out = feather_tk::Key::GraveAccent; break;
                case Qt::Key_Escape: out = feather_tk::Key::Escape; break;
                case Qt::Key_Enter: out = feather_tk::Key::Enter; break;
                case Qt::Key_Tab: out = feather_tk::Key::Tab; break;
                case Qt::Key_Backspace: out = feather_tk::Key::Backspace; break;
                case Qt::Key_Insert: out = feather_tk::Key::Insert; break;
                case Qt::Key_Delete: out = feather_tk::Key::Delete; break;
                case Qt::Key_Right: out = feather_tk::Key::Right; break;
                case Qt::Key_Left: out = feather_tk::Key::Left; break;
                case Qt::Key_Down: out = feather_tk::Key::Down; break;
                case Qt::Key_Up: out = feather_tk::Key::Up; break;
                case Qt::Key_PageUp: out = feather_tk::Key::PageUp; break;
                case Qt::Key_PageDown: out = feather_tk::Key::PageDown; break;
                case Qt::Key_Home: out = feather_tk::Key::Home; break;
                case Qt::Key_End: out = feather_tk::Key::End; break;
                case Qt::Key_CapsLock: out = feather_tk::Key::CapsLock; break;
                case Qt::Key_ScrollLock: out = feather_tk::Key::ScrollLock; break;
                case Qt::Key_NumLock: out = feather_tk::Key::NumLock; break;
                case Qt::Key_Print: out = feather_tk::Key::PrintScreen; break;
                case Qt::Key_Pause: out = feather_tk::Key::Pause; break;
                case Qt::Key_F1: out = feather_tk::Key::F1; break;
                case Qt::Key_F2: out = feather_tk::Key::F2; break;
                case Qt::Key_F3: out = feather_tk::Key::F3; break;
                case Qt::Key_F4: out = feather_tk::Key::F4; break;
                case Qt::Key_F5: out = feather_tk::Key::F5; break;
                case Qt::Key_F6: out = feather_tk::Key::F6; break;
                case Qt::Key_F7: out = feather_tk::Key::F7; break;
                case Qt::Key_F8: out = feather_tk::Key::F8; break;
                case Qt::Key_F9: out = feather_tk::Key::F9; break;
                case Qt::Key_F10: out = feather_tk::Key::F10; break;
                case Qt::Key_F11: out = feather_tk::Key::F11; break;
                case Qt::Key_F12: out = feather_tk::Key::F12; break;
                case Qt::Key_Shift: out = feather_tk::Key::LeftShift; break;
                case Qt::Key_Control: out = feather_tk::Key::LeftControl; break;
                case Qt::Key_Alt: out = feather_tk::Key::LeftAlt; break;
                case Qt::Key_Super_L: out = feather_tk::Key::LeftSuper; break;
                case Qt::Key_Super_R: out = feather_tk::Key::RightSuper; break;
                }
                return out;
            }
        }

        void ContainerWidget::keyPressEvent(QKeyEvent* event)
        {
            FEATHER_TK_P();
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
            FEATHER_TK_P();
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
            FEATHER_TK_P();
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

        feather_tk::V2I ContainerWidget::_toUI(const feather_tk::V2I& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        int ContainerWidget::_fromUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : 0.F;
        }

        feather_tk::V2I ContainerWidget::_fromUI(const feather_tk::V2I& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : feather_tk::V2I();
        }

        void ContainerWidget::_tickEvent()
        {
            FEATHER_TK_P();
            feather_tk::TickEvent tickEvent;
            _tickEvent(_p->window, true, true, tickEvent);
        }

        void ContainerWidget::_tickEvent(
            const std::shared_ptr<feather_tk::IWidget>& widget,
            bool visible,
            bool enabled,
            const feather_tk::TickEvent& event)
        {
            FEATHER_TK_P();
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

        bool ContainerWidget::_hasSizeUpdate(const std::shared_ptr<feather_tk::IWidget>& widget) const
        {
            bool out = widget->getUpdates() & static_cast<int>(feather_tk::Update::Size);
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
            FEATHER_TK_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            feather_tk::SizeHintEvent sizeHintEvent(
                p.fontSystem,
                p.iconSystem,
                devicePixelRatio,
                p.style);
            _sizeHintEvent(p.window, sizeHintEvent);
        }

        void ContainerWidget::_sizeHintEvent(
            const std::shared_ptr<feather_tk::IWidget>& widget,
            const feather_tk::SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEvent(child, event);
            }
            widget->sizeHintEvent(event);
        }

        void ContainerWidget::_setGeometry()
        {
            FEATHER_TK_P();
            const feather_tk::Box2I geometry(0, 0, _toUI(width()), _toUI(height()));
            p.window->setGeometry(geometry);
        }

        void ContainerWidget::_clipEvent()
        {
            FEATHER_TK_P();
            const feather_tk::Box2I geometry(0, 0, _toUI(width()), _toUI(height()));
            _clipEvent(p.window, geometry, false);
        }

        void ContainerWidget::_clipEvent(
            const std::shared_ptr<feather_tk::IWidget>& widget,
            const feather_tk::Box2I& clipRect,
            bool clipped)
        {
            const feather_tk::Box2I& g = widget->getGeometry();
            clipped |= !feather_tk::intersects(g, clipRect);
            clipped |= !widget->isVisible(false);
            const feather_tk::Box2I clipRect2 = feather_tk::intersect(g, clipRect);
            widget->clipEvent(clipRect2, clipped);
            const feather_tk::Box2I childrenClipRect =
                feather_tk::intersect(widget->getChildrenClipRect(), clipRect2);
            for (const auto& child : widget->getChildren())
            {
                const feather_tk::Box2I& childGeometry = child->getGeometry();
                _clipEvent(
                    child,
                    feather_tk::intersect(childGeometry, childrenClipRect),
                    clipped);
            }
        }

        bool ContainerWidget::_hasDrawUpdate(const std::shared_ptr<feather_tk::IWidget>& widget) const
        {
            bool out = false;
            if (!widget->isClipped())
            {
                out = widget->getUpdates() & static_cast<int>(feather_tk::Update::Draw);
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
            const std::shared_ptr<feather_tk::IWidget>& widget,
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            const feather_tk::Box2I& g = widget->getGeometry();
            if (!widget->isClipped() && g.w() > 0 && g.h() > 0)
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const feather_tk::Box2I childrenClipRect =
                    feather_tk::intersect(widget->getChildrenClipRect(), drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const feather_tk::Box2I& childGeometry = child->getGeometry();
                    if (feather_tk::intersects(childGeometry, childrenClipRect))
                    {
                        _drawEvent(
                            child,
                            feather_tk::intersect(childGeometry, childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }

        void ContainerWidget::_inputUpdate()
        {
            FEATHER_TK_P();
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
            /*FEATHER_TK_P();
            const auto palette = this->palette();
            p.style->setColorRole(
                feather_tk::ColorRole::Window,
                fromQt(palette.color(QPalette::ColorRole::Window)));
            p.style->setColorRole(
                feather_tk::ColorRole::Base,
                fromQt(palette.color(QPalette::ColorRole::Base)));
            p.style->setColorRole(
                feather_tk::ColorRole::Button,
                fromQt(palette.color(QPalette::ColorRole::Button)));
            p.style->setColorRole(
                feather_tk::ColorRole::Text,
                fromQt(palette.color(QPalette::ColorRole::WindowText)));*/
        }
    }
}
