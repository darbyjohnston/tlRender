// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineWidget.h"

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

                _fontSystem = imaging::FontSystem::create(context);

                _timer = startTimer(50);
            }

            TimelineWidget::~TimelineWidget()
            {}

            void TimelineWidget::setTimeline(const std::shared_ptr<timeline::Timeline>& timeline)
            {
                if (auto context = _context.lock())
                {
                    ItemData itemData;
                    itemData.fontSystem = _fontSystem;
                    itemData.fontMetrics = _fontSystem->getMetrics(itemData.fontInfo);
                    _timelineItem = TimelineItem::create(timeline, itemData, context);
                }
            }

            math::Vector2i TimelineWidget::timelineSize() const
            {
                math::Vector2i out;
                if (_timelineItem)
                {
                    out = _timelineItem->sizeHint();
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

            void TimelineWidget::initializeGL()
            {
                initializeOpenGLFunctions();
                gl::initGLAD();
                try
                {
                    if (auto context = _context.lock())
                    {
                        _render = gl::Render::create(context);
                    }

                }
                catch (const std::exception&)
                {}
            }

            void TimelineWidget::resizeGL(int w, int h)
            {}

            void TimelineWidget::paintGL()
            {
                if (_render)
                {
                    const float devicePixelRatio = window()->devicePixelRatio();
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
                    _render->end();
                }
            }

            void TimelineWidget::dragEnterEvent(QDragEnterEvent* event)
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
            }

            void TimelineWidget::timerEvent(QTimerEvent*)
            {
                if (_timelineItem)
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
                }
            }

            void TimelineWidget::_tick(const std::shared_ptr<BaseItem>& item)
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
            }
        }
    }
}
