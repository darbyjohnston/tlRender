// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineWidget.h>

#include <tlUI/EventLoop.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollArea.h>

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
            bool frameView = true;
            bool stopOnScrub = true;
            float mouseWheelScale = 20.F;
            ui::TimelineItemOptions itemOptions;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<ui::EventLoop> eventLoop;

            std::shared_ptr<ui::ScrollArea> scrollArea;
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

            setMouseTracking(true);
            setAcceptDrops(true);
            setFocusPolicy(Qt::StrongFocus);

            p.style = ui::Style::create(context);
            p.iconLibrary = ui::IconLibrary::create(context);
            p.fontSystem = imaging::FontSystem::create(context);
            p.eventLoop = ui::EventLoop::create(
                p.style,
                p.iconLibrary,
                p.fontSystem,
                context);
            p.scrollArea = ui::ScrollArea::create(context);
            p.eventLoop->addWidget(p.scrollArea);

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
                    ui::TimelineItemOptions options = p.itemOptions;
                    if (p.frameView)
                    {
                        _setScrollPos(math::Vector2i());
                        options.scale = _timelineScale();
                    }
                    _setItemOptions(p.timelineItem, options);
                    _setViewport(p.timelineItem, _timelineViewport());
                    p.timelineItem->setParent(p.scrollArea);
                }
            }
        }

        void TimelineWidget::setItemOptions(const ui::TimelineItemOptions& value)
        {
            TLRENDER_P();
            if (value == p.itemOptions)
                return;
            p.itemOptions = value;
            if (p.timelineItem)
            {
                ui::TimelineItemOptions options = p.itemOptions;
                if (p.frameView)
                {
                    _setScrollPos(math::Vector2i());
                    options.scale = _timelineScale();
                }
                _setItemOptions(p.timelineItem, options);
            }
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
                _getScrollPos());
        }

        void TimelineWidget::setFrameView(bool value)
        {
            TLRENDER_P();
            if (value == p.frameView)
                return;
            p.frameView = value;
            if (p.frameView && p.timelineItem)
            {
                _setScrollPos(math::Vector2i());
                ui::TimelineItemOptions options = p.itemOptions;
                options.scale = _timelineScale();
                _setItemOptions(p.timelineItem, options);
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
            p.eventLoop->setContentScale(devicePixelRatio);
            p.eventLoop->setSize(imaging::Size(
                w * devicePixelRatio,
                h * devicePixelRatio));
            if (p.timelineItem)
            {
                ui::TimelineItemOptions options = p.itemOptions;
                if (p.frameView)
                {
                    _setScrollPos(math::Vector2i());
                    options.scale = _timelineScale();
                }
                _setItemOptions(p.timelineItem, options);
                _setViewport(p.timelineItem, _timelineViewport());
            }
        }

        void TimelineWidget::paintGL()
        {
            TLRENDER_P();
            if (p.render)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                p.render->begin(imaging::Size(
                    width() * devicePixelRatio,
                    height() * devicePixelRatio));
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
                p.eventLoop->mouseButton(button, true, 0);
                break;
            }
            case Private::MouseMode::Scroll:
            case Private::MouseMode::Scale:
            {
                p.mouseScrollPos = _getScrollPos();
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
                p.eventLoop->mouseButton(button, false, 0);
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
                const float devicePixelRatio = window()->devicePixelRatio();
                p.eventLoop->cursorPos(
                    math::Vector2i(event->x(), event->y()) * devicePixelRatio);
                break;
            }
            case Private::MouseMode::Scroll:
            {
                const math::Vector2i d = p.mousePos - p.mousePressPos;
                _setScrollPos(p.mouseScrollPos - d);
                setFrameView(false);
                break;
            }
            case Private::MouseMode::Scale:
            {
                const float zoom = p.mouseScale + (p.mousePos.x - p.mousePressPos.x);
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
                p.mouseScrollPos = _getScrollPos();
                setViewZoom(zoom, p.mousePos);
            }
            p.mouseWheelTimer = now;
        }

        void TimelineWidget::keyPressEvent(QKeyEvent* event)
        {}

        void TimelineWidget::keyReleaseEvent(QKeyEvent*)
        {}

        void TimelineWidget::timerEvent(QTimerEvent*)
        {
            TLRENDER_P();
            p.eventLoop->tick();
            if (p.eventLoop->hasDrawUpdate())
            {
                update();
            }
        }

        math::Vector2i TimelineWidget::_getScrollPos() const
        {
            TLRENDER_P();
            const math::Vector2i& scrollPos = p.scrollArea->getScrollPos();
            const float devicePixelRatio = window()->devicePixelRatio();
            return devicePixelRatio > 0.F ? (scrollPos / devicePixelRatio) : math::Vector2i();
        }

        void TimelineWidget::_setScrollPos(const math::Vector2i& value)
        {
            TLRENDER_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            p.scrollArea->setScrollPos(value * devicePixelRatio);
        }

        void TimelineWidget::_setViewZoom(
            float zoomNew,
            float zoomPrev,
            const math::Vector2i& focus,
            const math::Vector2i& scrollPos)
        {
            TLRENDER_P();

            const int w = width();
            const int h = height();
            const float s = zoomNew / zoomPrev;
            const math::Vector2i scrollPosNew(
                (scrollPos.x + focus.x) * s - focus.x,
                scrollPos.y);
            _setScrollPos(scrollPosNew);

            p.itemOptions.scale = zoomNew;
            _setItemOptions(p.timelineItem, p.itemOptions);

            if (p.timelineItem)
            {
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
                    const float devicePixelRatio = window()->devicePixelRatio();
                    out = (width() - p.style->getSizeRole(ui::SizeRole::MarginSmall) * 2) *
                        devicePixelRatio / duration;
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
            TLRENDER_P();
            const math::Vector2i& scrollPos = p.scrollArea->getScrollPos();
            const float devicePixelRatio = window()->devicePixelRatio();
            return math::BBox2i(
                scrollPos.x,
                scrollPos.y,
                width() * devicePixelRatio,
                height() * devicePixelRatio);
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
    }
}
