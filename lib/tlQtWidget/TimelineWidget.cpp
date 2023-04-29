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

            setMinimumSize(16, 16);
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
                    _setScrollPos(math::Vector2i());
                    p.itemOptions.scale = _timelineScale();
                    _setItemOptions(p.timelineItem, p.itemOptions);
                    _setViewport(p.timelineItem, _timelineViewport());
                    p.timelineItem->setParent(p.scrollArea);
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
                _fromUI(p.scrollArea->getScrollPos()));
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
                _setScrollPos(math::Vector2i());
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

            const math::Vector2i scrollSize = _fromUI(p.scrollArea->getScrollSize());
            if (p.frameView || scrollSize.x < width())
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
                p.mouseScrollPos = _fromUI(p.scrollArea->getScrollPos());
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
                p.eventLoop->cursorPos(math::Vector2i(
                    _toUI(event->x()),
                    _toUI(event->y())));
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
                p.mouseScrollPos = _fromUI(p.scrollArea->getScrollPos());
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

        void TimelineWidget::_setScrollPos(const math::Vector2i& value)
        {
            TLRENDER_P();
            const int w = width();
            const int h = height();
            const math::Vector2i scrollSize = _fromUI(p.scrollArea->getScrollSize());
            const math::Vector2i clamped(
                math::clamp(value.x, 0, std::max(scrollSize.x - w, 0)),
                math::clamp(value.y, 0, std::max(scrollSize.y - h, 0)));
            p.scrollArea->setScrollPos(_toUI(clamped));
        }

        void TimelineWidget::_frameView()
        {
            TLRENDER_P();
            _setScrollPos(math::Vector2i());
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
            const math::Vector2i scrollSize = p.scrollArea->getScrollSize();
            const math::Vector2i scrollPosClamped(
                math::clamp(
                    scrollPosNew.x,
                    0,
                    std::max(static_cast<int>(scrollSize.x * s) - w, 0)),
                math::clamp(
                    scrollPosNew.y,
                    0,
                    std::max(static_cast<int>(scrollSize.y * s) - h, 0)));
            p.scrollArea->setScrollPos(scrollPosClamped);

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
                    const int m = p.style->getSizeRole(ui::SizeRole::MarginSmall, 1.F);
                    out = _toUI(width() - m * 2) / duration;
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
            return math::BBox2i(0, 0, _toUI(width()), _toUI(height()));
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
