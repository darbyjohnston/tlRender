// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/EventLoop.h>
#include <tlUI/IClipboard.h>
#include <tlUI/RowLayout.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Init.h>

#include <QClipboard>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QGuiApplication>
#include <QMimeData>

namespace tl
{
    namespace qtwidget
    {
        namespace
        {
            class Clipboard : public ui::IClipboard
            {
                TLRENDER_NON_COPYABLE(Clipboard);

            public:
                void _init(
                    const std::shared_ptr<system::Context>& context)
                {
                    IClipboard::_init(context);
                }

                Clipboard()
                {}

            public:
                ~Clipboard() override
                {}

                static std::shared_ptr<Clipboard> create(
                    const std::shared_ptr<system::Context>& context)
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

        struct TimelineWidget::Private
        {
            std::weak_ptr<system::Context> context;

            std::shared_ptr<timeline::Player> player;

            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<Clipboard> clipboard;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<ui::EventLoop> eventLoop;
            timelineui::ItemOptions itemOptions;
            std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
            std::chrono::steady_clock::time_point mouseWheelTimer;

            int timer = 0;

            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
        };

        TimelineWidget::TimelineWidget(
            const std::shared_ptr<ui::Style>& style,
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            //QSurfaceFormat surfaceFormat;
            //surfaceFormat.setMajorVersion(4);
            //surfaceFormat.setMinorVersion(1);
            //surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            //surfaceFormat.setStencilBufferSize(8);
            //setFormat(surfaceFormat);

            setMinimumSize(16, 16);
            setMouseTracking(true);
            setFocusPolicy(Qt::StrongFocus);

            p.style = style;
            p.iconLibrary = ui::IconLibrary::create(context);
            p.clipboard = Clipboard::create(context);
            p.eventLoop = ui::EventLoop::create(
                p.style,
                p.iconLibrary,
                p.clipboard,
                context);
            p.timelineWidget = timelineui::TimelineWidget::create(timeUnitsModel, context);
            //p.timelineWidget->setScrollBarsVisible(false);
            p.eventLoop->addWidget(p.timelineWidget);

            _styleUpdate();

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                p.timelineWidget->observeFrameView(),
                [this](bool value)
                {
                    Q_EMIT frameViewChanged(value);
                });

            p.timer = startTimer(10);
        }

        TimelineWidget::~TimelineWidget()
        {}

        void TimelineWidget::setPlayer(const std::shared_ptr<timeline::Player>& player)
        {
            TLRENDER_P();
            if (player == p.player)
                return;
            p.player = player;
            p.timelineWidget->setPlayer(p.player);
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->timelineWidget->hasFrameView();
        }

        bool TimelineWidget::areScrollBarsVisible() const
        {
            return _p->timelineWidget->areScrollBarsVisible();
        }

        ui::KeyModifier TimelineWidget::scrollKeyModifier() const
        {
            return _p->timelineWidget->getScrollKeyModifier();
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->timelineWidget->hasStopOnScrub();
        }

        float TimelineWidget::mouseWheelScale() const
        {
            return _p->timelineWidget->getMouseWheelScale();
        }

        const timelineui::ItemOptions& TimelineWidget::itemOptions() const
        {
            return _p->timelineWidget->getItemOptions();
        }

        void TimelineWidget::setFrameView(bool value)
        {
            _p->timelineWidget->setFrameView(value);
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->timelineWidget->setScrollBarsVisible(value);
        }

        void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
        {
            _p->timelineWidget->setScrollKeyModifier(value);
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            _p->timelineWidget->setStopOnScrub(value);
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            _p->timelineWidget->setMouseWheelScale(value);
        }

        void TimelineWidget::setItemOptions(const timelineui::ItemOptions& value)
        {
            _p->timelineWidget->setItemOptions(value);
        }

        void TimelineWidget::initializeGL()
        {
            TLRENDER_P();
            initializeOpenGLFunctions();
            gl::initGLAD();
            if (auto context = p.context.lock())
            {
                p.render = timeline::GLRender::create(context);
            }
        }

        void TimelineWidget::resizeGL(int w, int h)
        {
            TLRENDER_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            p.eventLoop->setDisplayScale(devicePixelRatio);
            p.eventLoop->setDisplaySize(imaging::Size(_toUI(w), _toUI(h)));
        }

        void TimelineWidget::paintGL()
        {
            TLRENDER_P();
            if (p.render)
            {
                timeline::RenderOptions renderOptions;
                renderOptions.clearColor = p.style->getColorRole(ui::ColorRole::Window);
                p.render->begin(
                    imaging::Size(_toUI(width()), _toUI(height())),
                    timeline::ColorConfigOptions(),
                    timeline::LUTOptions(),
                    renderOptions);
                p.eventLoop->draw(p.render);
                p.render->end();
            }
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        void TimelineWidget::enterEvent(QEvent* event)
#else
        void TimelineWidget::enterEvent(QEnterEvent* event)
#endif // QT_VERSION
        {
            TLRENDER_P();
            event->accept();
            p.eventLoop->cursorEnter(true);
        }

        void TimelineWidget::leaveEvent(QEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.eventLoop->cursorEnter(false);
        }

        namespace
        {
            int fromQtModifiers(int value)
            {
                int out = 0;
                if (value & Qt::ShiftModifier)
                {
                    out |= static_cast<int>(ui::KeyModifier::Shift);
                }
                if (value & Qt::ControlModifier)
                {
                    out |= static_cast<int>(ui::KeyModifier::Control);
                }
                if (value & Qt::AltModifier)
                {
                    out |= static_cast<int>(ui::KeyModifier::Alt);
                }
                return out;
            }
        }

        void TimelineWidget::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            event->accept();
            setFocus();
            int button = 0;
            if (event->button() == Qt::LeftButton)
            {
                button = 1;
            }
            p.eventLoop->mouseButton(
                button,
                true,
                fromQtModifiers(event->modifiers()));
        }

        void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            event->accept();
            int button = 0;
            if (event->button() == Qt::LeftButton)
            {
                button = 1;
            }
            p.eventLoop->mouseButton(
                button,
                false,
                fromQtModifiers(event->modifiers()));
        }

        void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.eventLoop->cursorPos(math::Vector2i(
                _toUI(event->x()),
                _toUI(event->y())));
        }

        void TimelineWidget::wheelEvent(QWheelEvent* event)
        {
            TLRENDER_P();
            const auto now = std::chrono::steady_clock::now();
            const auto diff = std::chrono::duration<float>(now - p.mouseWheelTimer);
            const float delta = event->angleDelta().y() / 8.F / 15.F;
            p.mouseWheelTimer = now;
            p.eventLoop->scroll(
                event->angleDelta().x() / 8.F / 15.F,
                event->angleDelta().y() / 8.F / 15.F,
                fromQtModifiers(event->modifiers()));
        }

        namespace
        {
            ui::Key fromQtKey(int key)
            {
                ui::Key out = ui::Key::Unknown;
                switch (key)
                {
                case Qt::Key_Space: out = ui::Key::Space; break;
                case Qt::Key_Apostrophe: out = ui::Key::Apostrophe; break;
                case Qt::Key_Comma: out = ui::Key::Comma; break;
                case Qt::Key_Minus: out = ui::Key::Minus; break;
                case Qt::Key_Period: out = ui::Key::Period; break;
                case Qt::Key_Slash: out = ui::Key::Slash; break;
                case Qt::Key_0: out = ui::Key::_0; break;
                case Qt::Key_1: out = ui::Key::_1; break;
                case Qt::Key_2: out = ui::Key::_2; break;
                case Qt::Key_3: out = ui::Key::_3; break;
                case Qt::Key_4: out = ui::Key::_4; break;
                case Qt::Key_5: out = ui::Key::_5; break;
                case Qt::Key_6: out = ui::Key::_6; break;
                case Qt::Key_7: out = ui::Key::_7; break;
                case Qt::Key_8: out = ui::Key::_8; break;
                case Qt::Key_9: out = ui::Key::_9; break;
                case Qt::Key_Semicolon: out = ui::Key::Semicolon; break;
                case Qt::Key_Equal: out = ui::Key::Equal; break;
                case Qt::Key_A: out = ui::Key::A; break;
                case Qt::Key_B: out = ui::Key::B; break;
                case Qt::Key_C: out = ui::Key::C; break;
                case Qt::Key_D: out = ui::Key::D; break;
                case Qt::Key_E: out = ui::Key::E; break;
                case Qt::Key_F: out = ui::Key::F; break;
                case Qt::Key_G: out = ui::Key::G; break;
                case Qt::Key_H: out = ui::Key::H; break;
                case Qt::Key_I: out = ui::Key::I; break;
                case Qt::Key_J: out = ui::Key::J; break;
                case Qt::Key_K: out = ui::Key::K; break;
                case Qt::Key_L: out = ui::Key::L; break;
                case Qt::Key_M: out = ui::Key::M; break;
                case Qt::Key_N: out = ui::Key::N; break;
                case Qt::Key_O: out = ui::Key::O; break;
                case Qt::Key_P: out = ui::Key::P; break;
                case Qt::Key_Q: out = ui::Key::Q; break;
                case Qt::Key_R: out = ui::Key::R; break;
                case Qt::Key_S: out = ui::Key::S; break;
                case Qt::Key_T: out = ui::Key::T; break;
                case Qt::Key_U: out = ui::Key::U; break;
                case Qt::Key_V: out = ui::Key::V; break;
                case Qt::Key_W: out = ui::Key::W; break;
                case Qt::Key_X: out = ui::Key::X; break;
                case Qt::Key_Y: out = ui::Key::Y; break;
                case Qt::Key_Z: out = ui::Key::Z; break;
                case Qt::Key_BracketLeft: out = ui::Key::LeftBracket; break;
                case Qt::Key_Backslash: out = ui::Key::Backslash; break;
                case Qt::Key_BracketRight: out = ui::Key::RightBracket; break;
                case Qt::Key_Agrave: out = ui::Key::GraveAccent; break;
                case Qt::Key_Escape: out = ui::Key::Escape; break;
                case Qt::Key_Enter: out = ui::Key::Enter; break;
                case Qt::Key_Tab: out = ui::Key::Tab; break;
                case Qt::Key_Backspace: out = ui::Key::Backspace; break;
                case Qt::Key_Insert: out = ui::Key::Insert; break;
                case Qt::Key_Delete: out = ui::Key::Delete; break;
                case Qt::Key_Right: out = ui::Key::Right; break;
                case Qt::Key_Left: out = ui::Key::Left; break;
                case Qt::Key_Down: out = ui::Key::Down; break;
                case Qt::Key_Up: out = ui::Key::Up; break;
                case Qt::Key_PageUp: out = ui::Key::PageUp; break;
                case Qt::Key_PageDown: out = ui::Key::PageDown; break;
                case Qt::Key_Home: out = ui::Key::Home; break;
                case Qt::Key_End: out = ui::Key::End; break;
                case Qt::Key_CapsLock: out = ui::Key::CapsLock; break;
                case Qt::Key_ScrollLock: out = ui::Key::ScrollLock; break;
                case Qt::Key_NumLock: out = ui::Key::NumLock; break;
                case Qt::Key_Print: out = ui::Key::PrintScreen; break;
                case Qt::Key_Pause: out = ui::Key::Pause; break;
                case Qt::Key_F1: out = ui::Key::F1; break;
                case Qt::Key_F2: out = ui::Key::F2; break;
                case Qt::Key_F3: out = ui::Key::F3; break;
                case Qt::Key_F4: out = ui::Key::F4; break;
                case Qt::Key_F5: out = ui::Key::F5; break;
                case Qt::Key_F6: out = ui::Key::F6; break;
                case Qt::Key_F7: out = ui::Key::F7; break;
                case Qt::Key_F8: out = ui::Key::F8; break;
                case Qt::Key_F9: out = ui::Key::F9; break;
                case Qt::Key_F10: out = ui::Key::F10; break;
                case Qt::Key_F11: out = ui::Key::F11; break;
                case Qt::Key_F12: out = ui::Key::F12; break;
                case Qt::Key_Shift: out = ui::Key::LeftShift; break;
                case Qt::Key_Control: out = ui::Key::LeftControl; break;
                case Qt::Key_Alt: out = ui::Key::LeftAlt; break;
                case Qt::Key_Super_L: out = ui::Key::LeftSuper; break;
                case Qt::Key_Super_R: out = ui::Key::RightSuper; break;
                }
                return out;
            }
        }

        void TimelineWidget::keyPressEvent(QKeyEvent* event)
        {
            TLRENDER_P();
            if (p.eventLoop->key(
                fromQtKey(event->key()),
                true,
                fromQtModifiers(event->modifiers())))
            {
                event->accept();
            }
        }

        void TimelineWidget::keyReleaseEvent(QKeyEvent* event)
        {
            TLRENDER_P();
            if (p.eventLoop->key(
                fromQtKey(event->key()),
                false,
                fromQtModifiers(event->modifiers())))
            {
                event->accept();
            }
        }

        void TimelineWidget::timerEvent(QTimerEvent*)
        {
            TLRENDER_P();
            p.eventLoop->tick();
            if (p.eventLoop->hasDrawUpdate())
            {
                update();
            }
        }

        bool TimelineWidget::event(QEvent* event)
        {
            TLRENDER_P();
            bool out = QOpenGLWidget::event(event);
            if (event->type() == QEvent::StyleChange)
            {
                _styleUpdate();
            }
            return out;
        }

        int TimelineWidget::_toUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        math::Vector2i TimelineWidget::_toUI(const math::Vector2i& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return value * devicePixelRatio;
        }

        int TimelineWidget::_fromUI(int value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : 0.F;
        }

        math::Vector2i TimelineWidget::_fromUI(const math::Vector2i& value) const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : math::Vector2i();
        }

        void TimelineWidget::_styleUpdate()
        {
            /*TLRENDER_P();
            const auto palette = this->palette();
            p.style->setColorRole(
                ui::ColorRole::Window,
                fromQt(palette.color(QPalette::ColorRole::Window)));
            p.style->setColorRole(
                ui::ColorRole::Base,
                fromQt(palette.color(QPalette::ColorRole::Base)));
            p.style->setColorRole(
                ui::ColorRole::Button,
                fromQt(palette.color(QPalette::ColorRole::Button)));
            p.style->setColorRole(
                ui::ColorRole::Text,
                fromQt(palette.color(QPalette::ColorRole::WindowText)));*/
        }
    }
}
