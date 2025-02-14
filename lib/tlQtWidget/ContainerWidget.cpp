// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/ContainerWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineGL/Render.h>

#include <dtk/ui/IClipboard.h>
#include <dtk/ui/IWindow.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/IconSystem.h>
#include <dtk/gl/Init.h>
#include <dtk/gl/Mesh.h>
#include <dtk/gl/OffscreenBuffer.h>
#include <dtk/gl/Shader.h>
#include <dtk/core/Context.h>

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

            class ContainerWindow : public dtk::IWindow
            {
                DTK_NON_COPYABLE(ContainerWindow);

            public:
                void _init(const std::shared_ptr<dtk::Context>& context)
                {
                    IWindow::_init(context, "tl::qtwidget::ContainerWindow", nullptr);
                }

                ContainerWindow()
                {}

            public:
                virtual ~ContainerWindow()
                {}

                static std::shared_ptr<ContainerWindow> create(
                    const std::shared_ptr<dtk::Context>& context)
                {
                    auto out = std::shared_ptr<ContainerWindow>(new ContainerWindow);
                    out->_init(context);
                    return out;
                }

                bool key(dtk::Key key, bool press, int modifiers)
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

                void cursorPos(const dtk::V2I& value)
                {
                    _cursorPos(value);
                }

                void mouseButton(int button, bool press, int modifiers)
                {
                    _mouseButton(button, press, modifiers);
                }

                void scroll(const dtk::V2F& value, int modifiers)
                {
                    _scroll(value, modifiers);
                }

                void setGeometry(const dtk::Box2I& value) override
                {
                    IWindow::setGeometry(value);
                    for (const auto& i : getChildren())
                    {
                        i->setGeometry(value);
                    }
                }
            };

            class Clipboard : public dtk::IClipboard
            {
                DTK_NON_COPYABLE(Clipboard);

            public:
                void _init(const std::shared_ptr<dtk::Context>& context)
                {
                    IClipboard::_init(context);
                }

                Clipboard()
                {}

            public:
                virtual ~Clipboard()
                {}

                static std::shared_ptr<Clipboard> create(
                    const std::shared_ptr<dtk::Context>& context)
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
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<dtk::Style> style;
            std::shared_ptr<dtk::IconSystem> iconSystem;
            std::shared_ptr<dtk::FontSystem> fontSystem;
            std::shared_ptr<Clipboard> clipboard;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<dtk::IWidget> widget;
            std::shared_ptr<ContainerWindow> window;
            std::shared_ptr<dtk::gl::Shader> shader;
            std::shared_ptr<dtk::gl::OffscreenBuffer> buffer;
            std::shared_ptr<dtk::gl::VBO> vbo;
            std::shared_ptr<dtk::gl::VAO> vao;
            bool inputEnabled = true;
            std::chrono::steady_clock::time_point mouseWheelTimer;
            std::unique_ptr<QTimer> timer;
        };

        ContainerWidget::ContainerWidget(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Style>& style,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.context = context;

            //QSurfaceFormat surfaceFormat;
            //surfaceFormat.setMajorVersion(4);
            //surfaceFormat.setMinorVersion(1);
            //surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            //surfaceFormat.setStencilBufferSize(8);
            //setFormat(surfaceFormat);

            p.style = style;
            p.iconSystem = context->getSystem<dtk::IconSystem>();
            p.fontSystem = context->getSystem<dtk::FontSystem>();
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

        const std::shared_ptr<dtk::IWidget>& ContainerWidget::getWidget() const
        {
            return _p->widget;
        }

        void ContainerWidget::setWidget(const std::shared_ptr<dtk::IWidget>& widget)
        {
            DTK_P();
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
            DTK_P();
            if (value == p.inputEnabled)
                return;
            p.inputEnabled = value;
            _inputUpdate();
        }

        QSize ContainerWidget::minimumSizeHint() const
        {
            DTK_P();
            dtk::Size2I sizeHint;
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
            DTK_P();
            initializeOpenGLFunctions();
            dtk::gl::initGLAD();
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
                        "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
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
                    p.shader = dtk::gl::Shader::create(vertexSource, fragmentSource);
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::qtwidget::TimelineWidget",
                            e.what(),
                            dtk::LogType::Error);
                    }
                }
            }
            _sizeHintEvent();
        }

        void ContainerWidget::resizeGL(int w, int h)
        {
            DTK_P();
            _setGeometry();
            p.vao.reset();
            p.vbo.reset();
        }

        void ContainerWidget::paintGL()
        {
            DTK_P();
            const dtk::Size2I renderSize(_toUI(width()), _toUI(height()));
            if (_hasDrawUpdate(p.window))
            {
                try
                {
                    if (renderSize.isValid())
                    {
                        dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.color = dtk::ImageType::RGBA_U8;
                        if (dtk::gl::doCreate(p.buffer, renderSize, offscreenBufferOptions))
                        {
                            p.buffer = dtk::gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                        }
                    }
                    else
                    {
                        p.buffer.reset();
                    }

                    if (p.render && p.buffer)
                    {
                        dtk::gl::OffscreenBufferBinding binding(p.buffer);
                        dtk::RenderOptions renderOptions;
                        renderOptions.clearColor = p.style->getColorRole(dtk::ColorRole::Window);
                        p.render->begin(renderSize, renderOptions);
                        const float devicePixelRatio = window()->devicePixelRatio();
                        dtk::DrawEvent drawEvent(
                            p.fontSystem,
                            p.iconSystem,
                            devicePixelRatio,
                            p.style,
                            p.render);
                        p.render->setClipRectEnabled(true);
                        _drawEvent(p.window, dtk::Box2I(dtk::V2I(), renderSize), drawEvent);
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
                            dtk::LogType::Error);
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
                const auto pm = dtk::ortho(
                    0.F,
                    static_cast<float>(renderSize.w),
                    0.F,
                    static_cast<float>(renderSize.h),
                    -1.F,
                    1.F);
                p.shader->setUniform("transform.mvp", pm);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                const auto mesh = dtk::mesh(dtk::Box2I(0, 0, renderSize.w, renderSize.h));
                if (!p.vbo)
                {
                    p.vbo = dtk::gl::VBO::create(mesh.triangles.size() * 3, dtk::gl::VBOType::Pos2_F32_UV_U16);
                }
                if (p.vbo)
                {
                    p.vbo->copy(convert(mesh, dtk::gl::VBOType::Pos2_F32_UV_U16));
                }

                if (!p.vao && p.vbo)
                {
                    p.vao = dtk::gl::VAO::create(dtk::gl::VBOType::Pos2_F32_UV_U16, p.vbo->getID());
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
            DTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorEnter(true);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                p.window->cursorPos(
                    dtk::V2I(_toUI(event->x()), _toUI(event->y())));
#endif // QT_VERSION
            }
        }

        void ContainerWidget::leaveEvent(QEvent* event)
        {
            DTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(dtk::V2I(-1, -1));
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
                    out |= static_cast<int>(dtk::KeyModifier::Shift);
                }
                if (value & Qt::ControlModifier)
                {
                    out |= static_cast<int>(dtk::KeyModifier::Control);
                }
                if (value & Qt::AltModifier)
                {
                    out |= static_cast<int>(dtk::KeyModifier::Alt);
                }
                return out;
            }
        }

        void ContainerWidget::mousePressEvent(QMouseEvent* event)
        {
            DTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(
                    dtk::V2I(_toUI(event->x()), _toUI(event->y())));
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
            DTK_P();
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
            DTK_P();
            if (p.inputEnabled)
            {
                event->accept();
                p.window->cursorPos(
                    dtk::V2I(_toUI(event->x()), _toUI(event->y())));
            }
        }

        void ContainerWidget::wheelEvent(QWheelEvent* event)
        {
            DTK_P();
            if (p.inputEnabled)
            {
                const auto now = std::chrono::steady_clock::now();
                const auto diff = std::chrono::duration<float>(now - p.mouseWheelTimer);
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                p.mouseWheelTimer = now;
                p.window->scroll(
                    dtk::V2F(
                        event->angleDelta().x() / 8.F / 15.F,
                        event->angleDelta().y() / 8.F / 15.F),
                    fromQtModifiers(event->modifiers()));
            }
        }

        namespace
        {
            dtk::Key fromQtKey(int key)
            {
                dtk::Key out = dtk::Key::Unknown;
                switch (key)
                {
                case Qt::Key_Space: out = dtk::Key::Space; break;
                case Qt::Key_Apostrophe: out = dtk::Key::Apostrophe; break;
                case Qt::Key_Comma: out = dtk::Key::Comma; break;
                case Qt::Key_Minus: out = dtk::Key::Minus; break;
                case Qt::Key_Period: out = dtk::Key::Period; break;
                case Qt::Key_Slash: out = dtk::Key::Slash; break;
                case Qt::Key_0: out = dtk::Key::_0; break;
                case Qt::Key_1: out = dtk::Key::_1; break;
                case Qt::Key_2: out = dtk::Key::_2; break;
                case Qt::Key_3: out = dtk::Key::_3; break;
                case Qt::Key_4: out = dtk::Key::_4; break;
                case Qt::Key_5: out = dtk::Key::_5; break;
                case Qt::Key_6: out = dtk::Key::_6; break;
                case Qt::Key_7: out = dtk::Key::_7; break;
                case Qt::Key_8: out = dtk::Key::_8; break;
                case Qt::Key_9: out = dtk::Key::_9; break;
                case Qt::Key_Semicolon: out = dtk::Key::Semicolon; break;
                case Qt::Key_Equal: out = dtk::Key::Equal; break;
                case Qt::Key_A: out = dtk::Key::A; break;
                case Qt::Key_B: out = dtk::Key::B; break;
                case Qt::Key_C: out = dtk::Key::C; break;
                case Qt::Key_D: out = dtk::Key::D; break;
                case Qt::Key_E: out = dtk::Key::E; break;
                case Qt::Key_F: out = dtk::Key::F; break;
                case Qt::Key_G: out = dtk::Key::G; break;
                case Qt::Key_H: out = dtk::Key::H; break;
                case Qt::Key_I: out = dtk::Key::I; break;
                case Qt::Key_J: out = dtk::Key::J; break;
                case Qt::Key_K: out = dtk::Key::K; break;
                case Qt::Key_L: out = dtk::Key::L; break;
                case Qt::Key_M: out = dtk::Key::M; break;
                case Qt::Key_N: out = dtk::Key::N; break;
                case Qt::Key_O: out = dtk::Key::O; break;
                case Qt::Key_P: out = dtk::Key::P; break;
                case Qt::Key_Q: out = dtk::Key::Q; break;
                case Qt::Key_R: out = dtk::Key::R; break;
                case Qt::Key_S: out = dtk::Key::S; break;
                case Qt::Key_T: out = dtk::Key::T; break;
                case Qt::Key_U: out = dtk::Key::U; break;
                case Qt::Key_V: out = dtk::Key::V; break;
                case Qt::Key_W: out = dtk::Key::W; break;
                case Qt::Key_X: out = dtk::Key::X; break;
                case Qt::Key_Y: out = dtk::Key::Y; break;
                case Qt::Key_Z: out = dtk::Key::Z; break;
                case Qt::Key_BracketLeft: out = dtk::Key::LeftBracket; break;
                case Qt::Key_Backslash: out = dtk::Key::Backslash; break;
                case Qt::Key_BracketRight: out = dtk::Key::RightBracket; break;
                case Qt::Key_Agrave: out = dtk::Key::GraveAccent; break;
                case Qt::Key_Escape: out = dtk::Key::Escape; break;
                case Qt::Key_Enter: out = dtk::Key::Enter; break;
                case Qt::Key_Tab: out = dtk::Key::Tab; break;
                case Qt::Key_Backspace: out = dtk::Key::Backspace; break;
                case Qt::Key_Insert: out = dtk::Key::Insert; break;
                case Qt::Key_Delete: out = dtk::Key::Delete; break;
                case Qt::Key_Right: out = dtk::Key::Right; break;
                case Qt::Key_Left: out = dtk::Key::Left; break;
                case Qt::Key_Down: out = dtk::Key::Down; break;
                case Qt::Key_Up: out = dtk::Key::Up; break;
                case Qt::Key_PageUp: out = dtk::Key::PageUp; break;
                case Qt::Key_PageDown: out = dtk::Key::PageDown; break;
                case Qt::Key_Home: out = dtk::Key::Home; break;
                case Qt::Key_End: out = dtk::Key::End; break;
                case Qt::Key_CapsLock: out = dtk::Key::CapsLock; break;
                case Qt::Key_ScrollLock: out = dtk::Key::ScrollLock; break;
                case Qt::Key_NumLock: out = dtk::Key::NumLock; break;
                case Qt::Key_Print: out = dtk::Key::PrintScreen; break;
                case Qt::Key_Pause: out = dtk::Key::Pause; break;
                case Qt::Key_F1: out = dtk::Key::F1; break;
                case Qt::Key_F2: out = dtk::Key::F2; break;
                case Qt::Key_F3: out = dtk::Key::F3; break;
                case Qt::Key_F4: out = dtk::Key::F4; break;
                case Qt::Key_F5: out = dtk::Key::F5; break;
                case Qt::Key_F6: out = dtk::Key::F6; break;
                case Qt::Key_F7: out = dtk::Key::F7; break;
                case Qt::Key_F8: out = dtk::Key::F8; break;
                case Qt::Key_F9: out = dtk::Key::F9; break;
                case Qt::Key_F10: out = dtk::Key::F10; break;
                case Qt::Key_F11: out = dtk::Key::F11; break;
                case Qt::Key_F12: out = dtk::Key::F12; break;
                case Qt::Key_Shift: out = dtk::Key::LeftShift; break;
                case Qt::Key_Control: out = dtk::Key::LeftControl; break;
                case Qt::Key_Alt: out = dtk::Key::LeftAlt; break;
                case Qt::Key_Super_L: out = dtk::Key::LeftSuper; break;
                case Qt::Key_Super_R: out = dtk::Key::RightSuper; break;
                }
                return out;
            }
        }

        void ContainerWidget::keyPressEvent(QKeyEvent* event)
        {
            DTK_P();
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
            DTK_P();
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
            DTK_P();
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

        dtk::V2I ContainerWidget::_toUI(const dtk::V2I& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        int ContainerWidget::_fromUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : 0.F;
        }

        dtk::V2I ContainerWidget::_fromUI(const dtk::V2I& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : dtk::V2I();
        }

        void ContainerWidget::_tickEvent()
        {
            DTK_P();
            dtk::TickEvent tickEvent;
            _tickEvent(_p->window, true, true, tickEvent);
        }

        void ContainerWidget::_tickEvent(
            const std::shared_ptr<dtk::IWidget>& widget,
            bool visible,
            bool enabled,
            const dtk::TickEvent& event)
        {
            DTK_P();
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

        bool ContainerWidget::_hasSizeUpdate(const std::shared_ptr<dtk::IWidget>& widget) const
        {
            bool out = widget->getUpdates() & static_cast<int>(dtk::Update::Size);
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
            DTK_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            dtk::SizeHintEvent sizeHintEvent(
                p.fontSystem,
                p.iconSystem,
                devicePixelRatio,
                p.style);
            _sizeHintEvent(p.window, sizeHintEvent);
        }

        void ContainerWidget::_sizeHintEvent(
            const std::shared_ptr<dtk::IWidget>& widget,
            const dtk::SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEvent(child, event);
            }
            widget->sizeHintEvent(event);
        }

        void ContainerWidget::_setGeometry()
        {
            DTK_P();
            const dtk::Box2I geometry(0, 0, _toUI(width()), _toUI(height()));
            p.window->setGeometry(geometry);
        }

        void ContainerWidget::_clipEvent()
        {
            DTK_P();
            const dtk::Box2I geometry(0, 0, _toUI(width()), _toUI(height()));
            _clipEvent(p.window, geometry, false);
        }

        void ContainerWidget::_clipEvent(
            const std::shared_ptr<dtk::IWidget>& widget,
            const dtk::Box2I& clipRect,
            bool clipped)
        {
            const dtk::Box2I& g = widget->getGeometry();
            clipped |= !dtk::intersects(g, clipRect);
            clipped |= !widget->isVisible(false);
            const dtk::Box2I clipRect2 = dtk::intersect(g, clipRect);
            widget->clipEvent(clipRect2, clipped);
            const dtk::Box2I childrenClipRect =
                dtk::intersect(widget->getChildrenClipRect(), clipRect2);
            for (const auto& child : widget->getChildren())
            {
                const dtk::Box2I& childGeometry = child->getGeometry();
                _clipEvent(
                    child,
                    dtk::intersect(childGeometry, childrenClipRect),
                    clipped);
            }
        }

        bool ContainerWidget::_hasDrawUpdate(const std::shared_ptr<dtk::IWidget>& widget) const
        {
            bool out = false;
            if (!widget->isClipped())
            {
                out = widget->getUpdates() & static_cast<int>(dtk::Update::Draw);
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
            const std::shared_ptr<dtk::IWidget>& widget,
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            const dtk::Box2I& g = widget->getGeometry();
            if (!widget->isClipped() && g.w() > 0 && g.h() > 0)
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const dtk::Box2I childrenClipRect =
                    dtk::intersect(widget->getChildrenClipRect(), drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const dtk::Box2I& childGeometry = child->getGeometry();
                    if (dtk::intersects(childGeometry, childrenClipRect))
                    {
                        _drawEvent(
                            child,
                            dtk::intersect(childGeometry, childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }

        void ContainerWidget::_inputUpdate()
        {
            DTK_P();
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
            /*DTK_P();
            const auto palette = this->palette();
            p.style->setColorRole(
                dtk::ColorRole::Window,
                fromQt(palette.color(QPalette::ColorRole::Window)));
            p.style->setColorRole(
                dtk::ColorRole::Base,
                fromQt(palette.color(QPalette::ColorRole::Base)));
            p.style->setColorRole(
                dtk::ColorRole::Button,
                fromQt(palette.color(QPalette::ColorRole::Button)));
            p.style->setColorRole(
                dtk::ColorRole::Text,
                fromQt(palette.color(QPalette::ColorRole::WindowText)));*/
        }
    }
}
