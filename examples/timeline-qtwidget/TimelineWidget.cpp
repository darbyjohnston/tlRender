// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineWidget.h"

#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>

#include <tlGL/Render.h>
#include <tlGL/Util.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            TimelineWidget::TimelineWidget(
                const std::shared_ptr<system::Context>& context,
                QWidget* parent) :
                QOpenGLWidget(parent),
                _context(context)
            {
                QSurfaceFormat surfaceFormat;
                surfaceFormat.setMajorVersion(4);
                surfaceFormat.setMinorVersion(1);
                surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
                surfaceFormat.setStencilBufferSize(8);
                setFormat(surfaceFormat);

                setMouseTracking(true);
                setAcceptDrops(true);

                _style = ui::Style::create(context);
                _iconLibrary = ui::IconLibrary::create(context);
                _fontSystem = imaging::FontSystem::create(context);
                _eventLoop = ui::EventLoop::create(
                    _style,
                    _iconLibrary,
                    _fontSystem,
                    context);
                _scrollArea = ui::ScrollArea::create(context);
                _eventLoop->addWidget(_scrollArea);

                _scrollSizeObserver = observer::ValueObserver<math::Vector2i>::create(
                    _scrollArea->observeScrollSize(),
                    [this](const math::Vector2i& value)
                    {
                        _scrollSize = value;
                        const float devicePixelRatio = window()->devicePixelRatio();
                        if (devicePixelRatio > 0.F)
                        {
                            _scrollSize.x /= devicePixelRatio;
                            _scrollSize.y /= devicePixelRatio;
                        }
                        Q_EMIT scrollSizeChanged(_scrollSize);
                    });
                _scrollPosObserver = observer::ValueObserver<math::Vector2i>::create(
                    _scrollArea->observeScrollPos(),
                    [this](const math::Vector2i& value)
                    {
                        _scrollPos = value;
                        const float devicePixelRatio = window()->devicePixelRatio();
                        if (devicePixelRatio > 0.F)
                        {
                            _scrollPos.x /= devicePixelRatio;
                            _scrollPos.y /= devicePixelRatio;
                        }
                        Q_EMIT scrollPosChanged(_scrollPos);
                    });

                _timer = startTimer(10);
            }

            TimelineWidget::~TimelineWidget()
            {}

            const math::Vector2i& TimelineWidget::scrollSize() const
            {
                return _scrollSize;
            }

            const math::Vector2i& TimelineWidget::scrollPos() const
            {
                return _scrollPos;
            }

            void TimelineWidget::setTimelinePlayer(const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer)
            {
                if (_timelineItem)
                {
                    _timelineItem->setParent(nullptr);
                    _timelineItem.reset();
                }
                if (timelinePlayer)
                {
                    if (auto context = _context.lock())
                    {
                        ItemData itemData;
                        itemData.directory = timelinePlayer->getPath().getDirectory();
                        itemData.pathOptions = timelinePlayer->getOptions().pathOptions;
                        itemData.ioManager = IOManager::create(
                            timelinePlayer->getOptions().ioOptions,
                            context);

                        _timelineItem = TimelineItem::create(timelinePlayer, itemData, context);
                        _setViewport(_timelineItem, _timelineViewport());
                        _timelineItem->setParent(_scrollArea);
                    }
                }
            }

            void TimelineWidget::setItemOptions(const ItemOptions& value)
            {
                if (_timelineItem)
                {
                    _setItemOptions(_timelineItem, value);
                }
            }

            void TimelineWidget::setScrollPos(const math::Vector2i& value)
            {
                if (value == _scrollPos)
                    return;
                _scrollPos = value;
                const float devicePixelRatio = window()->devicePixelRatio();
                _scrollArea->setScrollPos(_scrollPos * devicePixelRatio);
                if (_timelineItem)
                {
                    _setViewport(_timelineItem, _timelineViewport());
                }
                update();
            }

            void TimelineWidget::setScrollPosX(int value)
            {
                math::Vector2i scrollPos = _scrollPos;
                scrollPos.x = value;
                setScrollPos(scrollPos);
            }

            void TimelineWidget::setScrollPosY(int value)
            {
                math::Vector2i scrollPos = _scrollPos;
                scrollPos.y = value;
                setScrollPos(scrollPos);
            }

            void TimelineWidget::initializeGL()
            {
                initializeOpenGLFunctions();
                gl::initGLAD();
                if (auto context = _context.lock())
                {
                    _render = gl::Render::create(context);
                }
            }

            void TimelineWidget::resizeGL(int w, int h)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                _eventLoop->setContentScale(devicePixelRatio);
                _eventLoop->setSize(imaging::Size(
                    w * devicePixelRatio,
                    h * devicePixelRatio));
                if (_timelineItem)
                {
                    _setViewport(_timelineItem, _timelineViewport());
                }
            }

            void TimelineWidget::paintGL()
            {
                if (_render)
                {
                    const float devicePixelRatio = window()->devicePixelRatio();
                    _render->begin(imaging::Size(
                        width() * devicePixelRatio,
                        height() * devicePixelRatio));
                    _eventLoop->draw(_render);
                    _render->end();
                }
            }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            void TimelineWidget::enterEvent(QEvent* event)
#else
            void TimelineWidget::enterEvent(QEnterEvent* event)
#endif // QT_VERSION
            {
                event->accept();
                _eventLoop->cursorEnter(true);
            }

            void TimelineWidget::leaveEvent(QEvent* event)
            {
                event->accept();
                _eventLoop->cursorEnter(false);
            }

            void TimelineWidget::mousePressEvent(QMouseEvent* event)
            {
                event->accept();
                int button = 0;
                if (event->button() == Qt::LeftButton)
                {
                    button = 1;
                }
                _eventLoop->mouseButton(button, true, 0);
            }

            void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
            {
                event->accept();
                int button = 0;
                if (event->button() == Qt::LeftButton)
                {
                    button = 1;
                }
                _eventLoop->mouseButton(button, false, 0);
            }

            void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
            {
                event->accept();
                const float devicePixelRatio = window()->devicePixelRatio();
                _eventLoop->cursorPos(math::Vector2i(
                    event->x() * devicePixelRatio,
                    event->y() * devicePixelRatio));
                /*_mousePos.x = event->x();
                _mousePos.y = event->y();
                if (_mousePressed)
                {
                    const int w = width();
                    const int h = height();
                    const math::Vector2i timelineSize = this->timelineSize();
                    math::Vector2i viewPos;
                    viewPos.x = math::clamp(
                        _viewPosMousePress.x - (_mousePos.x - _mousePress.x),
                        0,
                        std::max(timelineSize.x - w, 0));
                    viewPos.y = math::clamp(
                        _viewPosMousePress.y - (_mousePos.y - _mousePress.y),
                        0,
                        std::max(timelineSize.y - h, 0));
                    if (viewPos != _viewPos)
                    {
                        _viewPos = viewPos;
                        Q_EMIT viewPosChanged(_viewPos);
                        update();
                    }
                }*/
            }

            void TimelineWidget::wheelEvent(QWheelEvent* event)
            {}

            void TimelineWidget::timerEvent(QTimerEvent*)
            {
                _eventLoop->tick();
                if (_eventLoop->hasDrawUpdate())
                {
                    update();
                }
            }

            void TimelineWidget::_setItemOptions(
                const std::shared_ptr<ui::IWidget>& widget,
                const ItemOptions& value)
            {
                if (auto item = std::dynamic_pointer_cast<IItem>(widget))
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
                const float devicePixelRatio = window()->devicePixelRatio();
                return math::BBox2i(
                    _scrollPos.x,
                    _scrollPos.y,
                    width(),
                    height()) * devicePixelRatio;
            }

            void TimelineWidget::_setViewport(
                const std::shared_ptr<ui::IWidget>& widget,
                const math::BBox2i& vp)
            {
                if (auto item = std::dynamic_pointer_cast<IItem>(widget))
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
}
