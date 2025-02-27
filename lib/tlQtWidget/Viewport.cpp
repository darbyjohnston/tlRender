// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Viewport.h>

#include <tlTimelineUI/Viewport.h>

namespace tl
{
    namespace qtwidget
    {
        struct Viewport::Private
        {
            std::shared_ptr<timelineui::Viewport> viewport;
        };

        Viewport::Viewport(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<dtk::Style>& style,
            QWidget* parent) :
            ContainerWidget(context, style, parent),
            _p(new Private)
        {
            DTK_P();
            p.viewport = timelineui::Viewport::create(context);
            setWidget(p.viewport);
        }

        Viewport::~Viewport()
        {}

        dtk::ImageType Viewport::colorBuffer() const
        {
            return _p->viewport->getColorBuffer();
        }

        const dtk::V2I& Viewport::viewPos() const
        {
            return _p->viewport->getViewPos();
        }

        double Viewport::viewZoom() const
        {
            return _p->viewport->getViewZoom();
        }

        bool Viewport::hasFrameView() const
        {
            return _p->viewport->hasFrameView();
        }

        double Viewport::getFPS() const
        {
            return _p->viewport->getFPS();
        }

        size_t Viewport::getDroppedFrames() const
        {
            return _p->viewport->getDroppedFrames();
        }

        void Viewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            _p->viewport->setOCIOOptions(value);
        }

        void Viewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            _p->viewport->setLUTOptions(value);
        }

        void Viewport::setImageOptions(const std::vector<dtk::ImageOptions>& value)
        {
            _p->viewport->setImageOptions(value);
        }

        void Viewport::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            _p->viewport->setDisplayOptions(value);
        }

        void Viewport::setCompareOptions(const timeline::CompareOptions& value)
        {
            _p->viewport->setCompareOptions(value);
        }

        void Viewport::setBackgroundOptions(const timeline::BackgroundOptions& value)
        {
            _p->viewport->setBackgroundOptions(value);
        }

        void Viewport::setColorBuffer(dtk::ImageType value)
        {
            _p->viewport->setColorBuffer(value);
        }

        void Viewport::setPlayer(const QSharedPointer<qt::PlayerObject>& value)
        {
            _p->viewport->setPlayer(value ? value->player() : nullptr);
        }

        void Viewport::setViewPosAndZoom(const dtk::V2I& pos, double zoom)
        {
            _p->viewport->setViewPosAndZoom(pos, zoom);
        }

        void Viewport::setViewZoom(double zoom, const dtk::V2I& focus)
        {
            _p->viewport->setViewZoom(zoom, focus);
        }

        void Viewport::setFrameView(bool value)
        {
            _p->viewport->setFrameView(value);
        }
        
        void Viewport::viewZoomReset()
        {
            _p->viewport->viewZoomReset();
        }

        void Viewport::viewZoomIn()
        {
            _p->viewport->viewZoomIn();
        }

        void Viewport::viewZoomOut()
        {
            _p->viewport->viewZoomOut();
        }
    }
}
