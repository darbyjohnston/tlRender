// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineWidget.h>

#include <tlUI/EventLoop.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

#include <tlGL/Render.h>
#include <tlGL/Util.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>

namespace tl
{
    namespace qtwidget
    {
        struct TimelineWidget::Private
        {
            std::weak_ptr<system::Context> context;

            std::shared_ptr<timeline::TimelinePlayer> timelinePlayer;
            bool timelinePlayerInit = false;
            bool frameView = true;
            bool stopOnScrub = true;
            float mouseWheelScale = 20.F;
            ui::TimelineItemOptions itemOptions;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<ui::EventLoop> eventLoop;

            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<ui::TimelineItem> timelineItem;

            bool mouseInside = false;
            math::Vector2i mousePos;
            math::Vector2i mousePressPos;
            enum class MouseMode
            {
                EventLoop,
                Scroll,
                Scale
            };
            MouseMode mouseMode = MouseMode::EventLoop;
            math::Vector2i mouseScrollPos;
            float mouseScale = 1.F;
            std::chrono::steady_clock::time_point mouseWheelTimer;

            int timer = 0;
        };

        TimelineWidget::TimelineWidget(
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            surfaceFormat.setStencilBufferSize(8);
            setFormat(surfaceFormat);

            setMinimumSize(16, 16);
            setMouseTracking(true);
            setFocusPolicy(Qt::StrongFocus);

            p.style = ui::Style::create(context);
            p.iconLibrary = ui::IconLibrary::create(context);
            p.fontSystem = imaging::FontSystem::create(context);
            p.eventLoop = ui::EventLoop::create(
                p.style,
                p.iconLibrary,
                p.fontSystem,
                context);
            p.scrollWidget = ui::ScrollWidget::create(context);
            p.scrollWidget->setBackgroundRole(ui::ColorRole::Window);
            p.scrollWidget->setScrollPosCallback(
                [this](const math::Vector2i&)
                {
                    _p->frameView = false;
                });
            p.eventLoop->addWidget(p.scrollWidget);

            p.timer = startTimer(10);
        }

        TimelineWidget::~TimelineWidget()
        {}

        void TimelineWidget::setTimelinePlayer(const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer)
        {
            TLRENDER_P();
            if (timelinePlayer == p.timelinePlayer)
                return;
            if (p.timelineItem)
            {
                p.timelineItem->setParent(nullptr);
                p.timelineItem.reset();
            }
            p.timelinePlayer = timelinePlayer;
            p.timelinePlayerInit = true;
            if (p.timelinePlayer)
            {
                if (auto context = p.context.lock())
                {
                    ui::TimelineItemData itemData;
                    itemData.directory = p.timelinePlayer->getPath().getDirectory();
                    itemData.pathOptions = p.timelinePlayer->getOptions().pathOptions;
                    itemData.ioManager = ui::TimelineIOManager::create(
                        p.timelinePlayer->getOptions().ioOptions,
                        context);

                    p.timelineItem = ui::TimelineItem::create(p.timelinePlayer, itemData, context);
                    p.timelineItem->setStopOnScrub(p.stopOnScrub);
                    p.scrollWidget->setWidget(p.timelineItem);
                }
            }
        }

        const ui::TimelineItemOptions& TimelineWidget::itemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setViewZoom(float value)
        {
            setViewZoom(value, math::Vector2i(width() / 2, height() / 2));
        }

        void TimelineWidget::setViewZoom(
            float zoom,
            const math::Vector2i& focus)
        {
            TLRENDER_P();
            _setViewZoom(
                zoom,
                p.itemOptions.scale,
                focus,
                _fromUI(p.scrollWidget->getScrollPos()));
        }

        void TimelineWidget::setFrameView(bool value)
        {
            TLRENDER_P();
            if (value == p.frameView)
                return;
            p.frameView = value;
            if (p.frameView)
            {
                _frameView();
            }
            Q_EMIT frameViewChanged(p.frameView);
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            TLRENDER_P();
            p.stopOnScrub = value;
            if (p.timelineItem)
            {
                p.timelineItem->setStopOnScrub(p.stopOnScrub);
            }
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            TLRENDER_P();
            p.mouseWheelScale = value;
        }

        void TimelineWidget::setItemOptions(const ui::TimelineItemOptions& value)
        {
            TLRENDER_P();
            if (value == p.itemOptions)
                return;
            p.itemOptions = value;
            if (p.frameView)
            {
                p.scrollWidget->setScrollPos(_toUI(math::Vector2i()));
                p.itemOptions.scale = _timelineScale();
            }
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
            }
        }

        void TimelineWidget::initializeGL()
        {
            TLRENDER_P();
            initializeOpenGLFunctions();
            gl::initGLAD();
            if (auto context = p.context.lock())
            {
                p.render = gl::Render::create(context);
            }
        }

        void TimelineWidget::resizeGL(int w, int h)
        {
            TLRENDER_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            p.eventLoop->setDisplayScale(devicePixelRatio);
            p.eventLoop->setDisplaySize(imaging::Size(_toUI(w), _toUI(h)));

            if (p.frameView)
            {
                _frameView();
            }
            if (p.timelineItem)
            {
                _setViewport(p.timelineItem, _timelineViewport());
            }
        }

        void TimelineWidget::paintGL()
        {
            TLRENDER_P();
            if (p.render)
            {
                p.render->begin(imaging::Size(
                    _toUI(width()),
                    _toUI(height())));
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
            p.mouseInside = true;
            p.eventLoop->cursorEnter(true);
        }

        void TimelineWidget::leaveEvent(QEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.mouseInside = false;
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
            p.mousePressPos = p.mousePos;
            if (event->modifiers() & Qt::ControlModifier)
            {
                p.mouseMode = Private::MouseMode::Scroll;
            }
            else if (event->modifiers() & Qt::AltModifier)
            {
                p.mouseMode = Private::MouseMode::Scale;
            }
            else
            {
                p.mouseMode = Private::MouseMode::EventLoop;
            }
            switch (p.mouseMode)
            {
            case Private::MouseMode::EventLoop:
            {
                int button = 0;
                if (event->button() == Qt::LeftButton)
                {
                    button = 1;
                }
                p.eventLoop->mouseButton(
                    button,
                    true,
                    fromQtModifiers(event->modifiers()));
                break;
            }
            case Private::MouseMode::Scroll:
            case Private::MouseMode::Scale:
            {
                p.mouseScrollPos = _fromUI(p.scrollWidget->getScrollPos());
                p.mouseScale = p.itemOptions.scale;
                break;
            }
            }
        }

        void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            event->accept();
            switch (p.mouseMode)
            {
            case Private::MouseMode::EventLoop:
            {
                int button = 0;
                if (event->button() == Qt::LeftButton)
                {
                    button = 1;
                }
                p.eventLoop->mouseButton(
                    button,
                    false,
                    fromQtModifiers(event->modifiers()));
                break;
            }
            case Private::MouseMode::Scroll:
                break;
            case Private::MouseMode::Scale:
                break;
            }
            p.mouseMode = Private::MouseMode::EventLoop;
        }

        void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.mousePos = math::Vector2i(event->x(), event->y());
            switch (p.mouseMode)
            {
            case Private::MouseMode::EventLoop:
            {
                p.eventLoop->cursorPos(math::Vector2i(
                    _toUI(event->x()),
                    _toUI(event->y())));
                break;
            }
            case Private::MouseMode::Scroll:
            {
                const math::Vector2i d = p.mousePos - p.mousePressPos;
                p.scrollWidget->setScrollPos(_toUI(p.mouseScrollPos - d));
                setFrameView(false);
                break;
            }
            case Private::MouseMode::Scale:
            {
                const float zoom = p.mouseScale + (p.mousePos.x - p.mousePressPos.x) * 10.F;
                _setViewZoom(
                    zoom,
                    p.mouseScale,
                    p.mousePressPos,
                    p.mouseScrollPos);
                break;
            }
            }
        }

        void TimelineWidget::wheelEvent(QWheelEvent* event)
        {
            TLRENDER_P();
            const auto now = std::chrono::steady_clock::now();
            const auto diff = std::chrono::duration<float>(now - p.mouseWheelTimer);
            const float delta = event->angleDelta().y() / 8.F / 15.F;
            const float zoom = p.itemOptions.scale + delta * p.mouseWheelScale;
            if (diff.count() < 1.0)
            {
                _setViewZoom(
                    zoom,
                    p.mouseScale,
                    p.mousePressPos,
                    p.mouseScrollPos);
            }
            else
            {
                p.mouseScale = p.itemOptions.scale;
                p.mousePressPos = p.mousePos;
                p.mouseScrollPos = _fromUI(p.scrollWidget->getScrollPos());
                setViewZoom(zoom, p.mousePos);
            }
            p.mouseWheelTimer = now;
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
            switch (event->key())
            {
            case Qt::Key::Key_0:
                event->accept();
                setViewZoom(1.F, p.mousePos);
                break;
            case Qt::Key::Key_Minus:
                event->accept();
                setViewZoom(p.itemOptions.scale / 2.F, p.mousePos);
                break;
            case Qt::Key::Key_Equal:
            case Qt::Key::Key_Plus:
                event->accept();
                setViewZoom(p.itemOptions.scale * 2.F, p.mousePos);
                break;
            case Qt::Key::Key_Backspace:
                event->accept();
                _frameView();
                break;
            default:
                event->accept();
                p.eventLoop->key(
                    fromQtKey(event->key()),
                    true,
                    fromQtModifiers(event->modifiers()));
                break;
            }
        }

        void TimelineWidget::keyReleaseEvent(QKeyEvent* event)
        {
            TLRENDER_P();
            switch (event->key())
            {
            case Qt::Key::Key_0:
            case Qt::Key::Key_Minus:
            case Qt::Key::Key_Equal:
            case Qt::Key::Key_Plus:
            case Qt::Key::Key_Backspace:
                event->accept();
                break;
            default:
                event->accept();
                p.eventLoop->key(
                    fromQtKey(event->key()),
                    false,
                    fromQtModifiers(event->modifiers()));
                break;
            }
        }

        void TimelineWidget::timerEvent(QTimerEvent*)
        {
            TLRENDER_P();
            p.eventLoop->tick();
            if (p.timelinePlayerInit)
            {
                p.timelinePlayerInit = false;
                if (p.timelineItem)
                {
                    p.scrollWidget->setScrollPos(_toUI(math::Vector2i()));
                    p.itemOptions.scale = _timelineScale();
                    _setItemOptions(p.timelineItem, p.itemOptions);
                    _setViewport(p.timelineItem, _timelineViewport());
                }
            }
            if (p.eventLoop->hasDrawUpdate())
            {
                update();
            }
        }

        void TimelineWidget::_frameView()
        {
            TLRENDER_P();
            p.scrollWidget->setScrollPos(_toUI(math::Vector2i()));
            p.itemOptions.scale = _timelineScale();
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
            }
        }

        void TimelineWidget::_setViewZoom(
            float zoomNew,
            float zoomPrev,
            const math::Vector2i& focus,
            const math::Vector2i& scrollPos)
        {
            TLRENDER_P();

            const int w = _toUI(width());
            const int h = _toUI(height());
            const float zoomMin = _timelineScale();
            const float zoomMax = w;
            const float zoomClamped = math::clamp(zoomNew, zoomMin, zoomMax);

            const float s = zoomClamped / zoomPrev;
            const math::Vector2i scrollPosNew = _toUI(math::Vector2i(
                (scrollPos.x + focus.x) * s - focus.x,
                scrollPos.y));
            const math::Vector2i scrollSize = p.scrollWidget->getScrollSize();
            const math::Vector2i scrollPosClamped(
                math::clamp(
                    scrollPosNew.x,
                    0,
                    std::max(static_cast<int>(scrollSize.x * s) - w, 0)),
                math::clamp(
                    scrollPosNew.y,
                    0,
                    std::max(static_cast<int>(scrollSize.y * s) - h, 0)));
            p.scrollWidget->setScrollPos(scrollPosClamped);

            p.itemOptions.scale = zoomClamped;
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
                _setViewport(p.timelineItem, _timelineViewport());
            }

            setFrameView(false);
        }

        float TimelineWidget::_timelineScale() const
        {
            TLRENDER_P();
            float out = 100.F;
            if (p.timelinePlayer)
            {
                const otime::TimeRange& timeRange = p.timelinePlayer->getTimeRange();
                const double duration = timeRange.duration().rescaled_to(1.0).value();
                if (duration > 0.0)
                {
                    const math::Vector2i& scrollAreaSize = p.scrollWidget->getScrollAreaSize();
                    const float devicePixelRatio = window()->devicePixelRatio();
                    const int m = p.style->getSizeRole(ui::SizeRole::MarginSmall, devicePixelRatio);
                    out = (scrollAreaSize.x - m * 2) / duration;
                }
            }
            return out;
        }

        void TimelineWidget::_setItemOptions(
            const std::shared_ptr<ui::IWidget>& widget,
            const ui::TimelineItemOptions& value)
        {
            if (auto item = std::dynamic_pointer_cast<ui::ITimelineItem>(widget))
            {
                item->setOptions(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setItemOptions(child, value);
            }
        }

        math::BBox2i TimelineWidget::_timelineViewport() const
        {
            return _p->scrollWidget->getScrollAreaGeometry();
        }

        void TimelineWidget::_setViewport(
            const std::shared_ptr<ui::IWidget>& widget,
            const math::BBox2i& vp)
        {
            if (auto item = std::dynamic_pointer_cast<ui::ITimelineItem>(widget))
            {
                item->setViewport(vp);
            }
            for (const auto& child : widget->getChildren())
            {
                _setViewport(child, vp);
            }
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
    }
}
