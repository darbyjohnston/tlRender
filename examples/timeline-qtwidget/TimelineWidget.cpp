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

                _timer = startTimer(10);
            }

            TimelineWidget::~TimelineWidget()
            {}

            void TimelineWidget::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
            {
                if (auto context = _context.lock())
                {
                    _timelineItem = TimelineItem::create(timeline, context);
                    _eventLoop->addWidget(_timelineItem);
                }
                //    ItemData itemData;
                //    itemData.fontSystem = _fontSystem;
                //    itemData.fontMetrics = _fontSystem->getMetrics(itemData.fontInfo);
                //    _timelineItem = TimelineItem::create(timeline, itemData, context);
                //}
            }

            math::Vector2i TimelineWidget::timelineSize() const
            {
                math::Vector2i out;
                if (_timelineItem)
                {
                    out = _timelineItem->getSizeHint();
                }
                return out;
            }

            void TimelineWidget::setScale(float value)
            {
                if (_timelineItem)
                {
                    _timelineItem->setScale(value);
                }
            }

            void TimelineWidget::setThumbnailHeight(int value)
            {
                if (_timelineItem)
                {
                    _timelineItem->setThumbnailHeight(value);
                }
            }

            void TimelineWidget::setViewPos(const math::Vector2i& value)
            {
                if (value == _viewPos)
                    return;
                _viewPos = value;
                update();
            }

            void TimelineWidget::setViewPosX(int value)
            {
                if (value == _viewPos.x)
                    return;
                _viewPos.x = value;
                update();
            }

            void TimelineWidget::setViewPosY(int value)
            {
                if (value == _viewPos.y)
                    return;
                _viewPos.y = value;
                update();
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

                    /*const float devicePixelRatio = window()->devicePixelRatio();
                    _render->begin(imaging::Size(
                        width() * devicePixelRatio,
                        height() * devicePixelRatio));
                    if (_timelineItem)
                    {
                        _renderItems(
                            _timelineItem,
                            _render,
                            math::BBox2i(_viewPos.x, _viewPos.y, width(), height()),
                            devicePixelRatio);
                    }
                    _render->end();*/
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

            /*void TimelineWidget::dragEnterEvent(QDragEnterEvent* event)
            {
                const QMimeData* mimeData = event->mimeData();
                if (mimeData->hasUrls())
                {
                    event->acceptProposedAction();
                }
            }

            void TimelineWidget::dragMoveEvent(QDragMoveEvent* event)
            {
                const QMimeData* mimeData = event->mimeData();
                if (mimeData->hasUrls())
                {
                    event->acceptProposedAction();
                }
            }

            void TimelineWidget::dragLeaveEvent(QDragLeaveEvent* event)
            {
                event->accept();
            }

            void TimelineWidget::dropEvent(QDropEvent* event)
            {
                const QMimeData* mimeData = event->mimeData();
                if (mimeData->hasUrls())
                {
                    const auto urlList = mimeData->urls();
                    for (int i = 0; i < urlList.size(); ++i)
                    {
                    }
                }
            }*/

            void TimelineWidget::timerEvent(QTimerEvent*)
            {
                /*if (_timelineItem)
                {
                    _tick(_timelineItem);
                    if (_doLayout(_timelineItem))
                    {
                        _preLayout(_timelineItem);
                        const auto& sizeHint = _timelineItem->sizeHint();
                        _timelineItem->layout(math::BBox2i(0, 0, sizeHint.x, sizeHint.y));
                        if (_timelineSize != sizeHint)
                        {
                            _timelineSize = sizeHint;
                            Q_EMIT timelineSizeChanged(_timelineSize);
                        }
                    }
                    if (_doRender(_timelineItem))
                    {
                        update();
                    }
                }*/
                _eventLoop->tick();

                const math::Vector2i& sizeHint = _timelineItem->getSizeHint();
                if (sizeHint != _timelineSize)
                {
                    _timelineSize = sizeHint;
                    _timelineItem->setGeometry(math::BBox2i(0, 0, _timelineSize.x, _timelineSize.y));
                    Q_EMIT timelineSizeChanged(_timelineSize);
                }

                if (_eventLoop->hasDrawUpdate())
                {
                    update();
                }
            }

            /*void TimelineWidget::_tick(const std::shared_ptr<BaseItem>& item)
            {
                for (const auto& child : item->children())
                {
                    _tick(child);
                }
                item->tick();
            }

            bool TimelineWidget::_doLayout(const std::shared_ptr<BaseItem>& item)
            {
                bool out = false;
                for (const auto& child : item->children())
                {
                    out |= _doLayout(child);
                }
                out |= item->doLayout();
                return out;
            }

            void TimelineWidget::_preLayout(const std::shared_ptr<BaseItem>& item)
            {
                for (const auto& child : item->children())
                {
                    _preLayout(child);
                }
                item->preLayout();
            }

            bool TimelineWidget::_doRender(const std::shared_ptr<BaseItem>&item)
            {
                bool out = false;
                for (const auto& child : item->children())
                {
                    out |= _doRender(child);
                }
                out |= item->doRender();
                return out;
            }

            void TimelineWidget::_renderItems(
                const std::shared_ptr<BaseItem>& item,
                const std::shared_ptr<timeline::IRender>& render,
                const math::BBox2i& viewport,
                float devicePixelRatio)
            {
                item->render(render, viewport, devicePixelRatio);
                for (const auto& child : item->children())
                {
                    _renderItems(child, render, viewport, devicePixelRatio);
                }
            }*/
        }
    }
}
