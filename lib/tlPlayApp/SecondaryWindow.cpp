// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/SecondaryWindow.h>

#include <tlPlayApp/Models/ColorModel.h>
#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/Models/RenderModel.h>
#include <tlPlayApp/Models/ViewportModel.h>
#include <tlPlayApp/App.h>

#include <tlTimelineUI/Viewport.h>

namespace tl
{
    namespace play
    {
        struct SecondaryWindow::Private
        {
            std::shared_ptr<timelineui::Viewport> viewport;

            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
        };

        void SecondaryWindow::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::Window>& shared)
        {
            Window::_init(context, "tlplay 2", dtk::Size2I(1920, 1080));
            DTK_P();

            p.viewport = timelineui::Viewport::create(context);
            p.viewport->setParent(shared_from_this());

            p.playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _p->viewport->setPlayer(value);
                });

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->viewport->setCompareOptions(value);
                });

            p.ocioOptionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                app->getColorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                    _p->viewport->setOCIOOptions(value);
                });

            p.lutOptionsObserver = dtk::ValueObserver<timeline::LUTOptions>::create(
                app->getColorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _p->viewport->setLUTOptions(value);
                });

            p.imageOptionsObserver = dtk::ValueObserver<dtk::ImageOptions>::create(
                app->getRenderModel()->observeImageOptions(),
                [this](const dtk::ImageOptions& value)
                {
                    _p->viewport->setImageOptions({ value });
                });

            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->viewport->setDisplayOptions({ value });
                });

            p.backgroundOptionsObserver = dtk::ValueObserver<timeline::BackgroundOptions>::create(
                app->getViewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _p->viewport->setBackgroundOptions(value);
                });

            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                app->getRenderModel()->observeColorBuffer(),
                [this](dtk::ImageType value)
                {
                    _p->viewport->setColorBuffer(value);
                });
        }

        SecondaryWindow::SecondaryWindow() :
            _p(new Private)
        {}

        SecondaryWindow::~SecondaryWindow()
        {
            _makeCurrent();
            _p->viewport->setParent(nullptr);
        }

        std::shared_ptr<SecondaryWindow> SecondaryWindow::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::Window>& shared)
        {
            auto out = std::shared_ptr<SecondaryWindow>(new SecondaryWindow);
            out->_init(context, app, shared);
            return out;
        }

        const std::shared_ptr<timelineui::Viewport>& SecondaryWindow::getViewport() const
        {
            return _p->viewport;
        }

        void SecondaryWindow::setView(
            const dtk::V2I& pos,
            double zoom,
            bool frame)
        {
            DTK_P();
            p.viewport->setViewPosAndZoom(pos, zoom);
            p.viewport->setFrameView(frame);
        }
    }
}
