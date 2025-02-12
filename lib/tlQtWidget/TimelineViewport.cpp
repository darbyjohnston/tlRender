// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineViewport.h>

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace qtwidget
    {
        struct TimelineViewport::Private
        {
            std::shared_ptr<timelineui::TimelineViewport> viewport;
        };

        TimelineViewport::TimelineViewport(
            const std::shared_ptr<ui::Style>& style,
            const std::shared_ptr<dtk::Context>& context,
            QWidget* parent) :
            ContainerWidget(style, context, parent),
            _p(new Private)
        {
            DTK_P();
            p.viewport = timelineui::TimelineViewport::create(context);
            setWidget(p.viewport);
        }

        TimelineViewport::~TimelineViewport()
        {}

        dtk::ImageType TimelineViewport::colorBuffer() const
        {
            return _p->viewport->getColorBuffer();
        }

        const dtk::V2I& TimelineViewport::viewPos() const
        {
            return _p->viewport->getViewPos();
        }

        double TimelineViewport::viewZoom() const
        {
            return _p->viewport->getViewZoom();
        }

        bool TimelineViewport::hasFrameView() const
        {
            return _p->viewport->hasFrameView();
        }

        double TimelineViewport::getFPS() const
        {
            return _p->viewport->getFPS();
        }

        size_t TimelineViewport::getDroppedFrames() const
        {
            return _p->viewport->getDroppedFrames();
        }

        void TimelineViewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            _p->viewport->setOCIOOptions(value);
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            _p->viewport->setLUTOptions(value);
        }

        void TimelineViewport::setImageOptions(const std::vector<dtk::ImageOptions>& value)
        {
            _p->viewport->setImageOptions(value);
        }

        void TimelineViewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            _p->viewport->setDisplayOptions(value);
        }

        void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            _p->viewport->setCompareOptions(value);
        }

        void TimelineViewport::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            _p->viewport->setBackgroundOptions(value);
        }

        void TimelineViewport::setColorBuffer(dtk::ImageType value)
        {
            _p->viewport->setColorBuffer(value);
        }

        void TimelineViewport::setPlayer(const QSharedPointer<qt::TimelinePlayer>& value)
        {
            _p->viewport->setPlayer(value ? value->player() : nullptr);
        }

        void TimelineViewport::setViewPosAndZoom(const dtk::V2I& pos, double zoom)
        {
            _p->viewport->setViewPosAndZoom(pos, zoom);
        }

        void TimelineViewport::setViewZoom(double zoom, const dtk::V2I& focus)
        {
            _p->viewport->setViewZoom(zoom, focus);
        }

        void TimelineViewport::setFrameView(bool value)
        {
            _p->viewport->setFrameView(value);
        }
        
        void TimelineViewport::viewZoomReset()
        {
            _p->viewport->viewZoomReset();
        }

        void TimelineViewport::viewZoomIn()
        {
            _p->viewport->viewZoomIn();
        }

        void TimelineViewport::viewZoomOut()
        {
            _p->viewport->viewZoomOut();
        }
    }
}
