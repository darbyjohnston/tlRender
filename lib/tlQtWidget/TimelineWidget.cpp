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
            std::shared_ptr<imaging::FontSystem> fontSystem;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<ui::EventLoop> eventLoop;

            std::shared_ptr<ui::ScrollArea> scrollArea;
            math::Vector2i scrollSize;
            math::Vector2i scrollPos;
            std::shared_ptr<observer::ValueObserver<math::Vector2i> > scrollSizeObserver;
            std::shared_ptr<observer::ValueObserver<math::Vector2i> > scrollPosObserver;
            std::shared_ptr<ui::TimelineItem> timelineItem;

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

            p.scrollSizeObserver = observer::ValueObserver<math::Vector2i>::create(
                p.scrollArea->observeScrollSize(),
                [this](const math::Vector2i& value)
                {
                    _p->scrollSize = value;
                    const float devicePixelRatio = window()->devicePixelRatio();
                    if (devicePixelRatio > 0.F)
                    {
                        _p->scrollSize.x /= devicePixelRatio;
                        _p->scrollSize.y /= devicePixelRatio;
                    }
                    Q_EMIT scrollSizeChanged(_p->scrollSize);
                });
            p.scrollPosObserver = observer::ValueObserver<math::Vector2i>::create(
                p.scrollArea->observeScrollPos(),
                [this](const math::Vector2i& value)
                {
                    _p->scrollPos = value;
                    const float devicePixelRatio = window()->devicePixelRatio();
                    if (devicePixelRatio > 0.F)
                    {
                        _p->scrollPos.x /= devicePixelRatio;
                        _p->scrollPos.y /= devicePixelRatio;
                    }
                    Q_EMIT scrollPosChanged(_p->scrollPos);
                });

            p.timer = startTimer(10);
        }

        TimelineWidget::~TimelineWidget()
        {}

        const math::Vector2i& TimelineWidget::scrollSize() const
        {
            return _p->scrollSize;
        }

        const math::Vector2i& TimelineWidget::scrollPos() const
        {
            return _p->scrollPos;
        }

        void TimelineWidget::setTimelinePlayer(const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer)
        {
            TLRENDER_P();
            if (p.timelineItem)
            {
                p.timelineItem->setParent(nullptr);
                p.timelineItem.reset();
            }
            if (timelinePlayer)
            {
                if (auto context = p.context.lock())
                {
                    ui::TimelineItemData itemData;
                    itemData.directory = timelinePlayer->getPath().getDirectory();
                    itemData.pathOptions = timelinePlayer->getOptions().pathOptions;
                    itemData.ioManager = ui::TimelineIOManager::create(
                        timelinePlayer->getOptions().ioOptions,
                        context);

                    p.timelineItem = ui::TimelineItem::create(timelinePlayer, itemData, context);
                    _setViewport(p.timelineItem, _timelineViewport());
                    p.timelineItem->setParent(p.scrollArea);
                }
            }
        }

        void TimelineWidget::setItemOptions(const ui::TimelineItemOptions& value)
        {
            TLRENDER_P();
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, value);
            }
        }

        void TimelineWidget::setScrollPos(const math::Vector2i& value)
        {
            TLRENDER_P();
            if (value == p.scrollPos)
                return;
            p.scrollPos = value;
            const float devicePixelRatio = window()->devicePixelRatio();
            p.scrollArea->setScrollPos(p.scrollPos * devicePixelRatio);
            if (p.timelineItem)
            {
                _setViewport(p.timelineItem, _timelineViewport());
            }
            update();
        }

        void TimelineWidget::setScrollPosX(int value)
        {
            math::Vector2i scrollPos = _p->scrollPos;
            scrollPos.x = value;
            setScrollPos(scrollPos);
        }

        void TimelineWidget::setScrollPosY(int value)
        {
            math::Vector2i scrollPos = _p->scrollPos;
            scrollPos.y = value;
            setScrollPos(scrollPos);
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
            p.eventLoop->cursorEnter(true);
        }

        void TimelineWidget::leaveEvent(QEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.eventLoop->cursorEnter(false);
        }

        void TimelineWidget::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            event->accept();
            int button = 0;
            if (event->button() == Qt::LeftButton)
            {
                button = 1;
            }
            p.eventLoop->mouseButton(button, true, 0);
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
            p.eventLoop->mouseButton(button, false, 0);
        }

        void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            event->accept();
            const float devicePixelRatio = window()->devicePixelRatio();
            p.eventLoop->cursorPos(math::Vector2i(
                event->x() * devicePixelRatio,
                event->y() * devicePixelRatio));
        }

        void TimelineWidget::wheelEvent(QWheelEvent* event)
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
            const float devicePixelRatio = window()->devicePixelRatio();
            return math::BBox2i(
                p.scrollPos.x,
                p.scrollPos.y,
                width(),
                height()) * devicePixelRatio;
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
