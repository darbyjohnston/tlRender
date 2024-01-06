// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SecondaryWindow.h>

#include <tlPlayQtApp/App.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/Settings.h>
#include <tlPlay/ViewportModel.h>

#include <tlQtWidget/TimelineViewport.h>

#include <QBoxLayout>
#include <QKeyEvent>

namespace tl
{
    namespace play_qt
    {
        struct SecondaryWindow::Private
        {
            App* app = nullptr;

            qtwidget::TimelineViewport* viewport = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
        };

        SecondaryWindow::SecondaryWindow(
            App* app,
            QWidget* parent) :
            QWidget(parent),
            _p(new  Private)
        {
            TLRENDER_P();

            setAttribute(Qt::WA_DeleteOnClose);

            p.app = app;

            p.viewport = new qtwidget::TimelineViewport(app->getContext());

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.viewport);
            setLayout(layout);

            auto settings = app->settings();
            std::string geometry;
            settings->setDefaultValue("SecondaryWindow/Size", math::Size2i(1920, 1080));

            const math::Size2i size = settings->getValue<math::Size2i>("SecondaryWindow/Size");
            resize(size.w, size.h);

            p.viewport->setTimelinePlayers(app->activePlayers());

            connect(
                app,
                &App::activePlayersChanged,
                [this](const QVector<QSharedPointer<qt::TimelinePlayer> >& value)
                {
                    _p->viewport->setTimelinePlayers(value);
                });

            p.backgroundOptionsObserver = observer::ValueObserver<timeline::BackgroundOptions>::create(
                app->viewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _p->viewport->setBackgroundOptions(value);
                });

            p.ocioOptionsObserver = observer::ValueObserver<timeline::OCIOOptions>::create(
                app->colorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                    _p->viewport->setOCIOOptions(value);
                });

            p.lutOptionsObserver = observer::ValueObserver<timeline::LUTOptions>::create(
                app->colorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _p->viewport->setLUTOptions(value);
                });

            p.imageOptionsObserver = observer::ValueObserver<timeline::ImageOptions>::create(
                app->colorModel()->observeImageOptions(),
                [this](const timeline::ImageOptions& value)
                {
                    _p->viewport->setImageOptions({ value });
                });

            p.displayOptionsObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->colorModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->viewport->setDisplayOptions({ value });
                });

            p.compareOptionsObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->viewport->setCompareOptions(value);
                });
        }

        SecondaryWindow::~SecondaryWindow()
        {
            TLRENDER_P();
            const math::Size2i size(width(), height());
            p.app->settings()->setValue("SecondaryWindow/Size", size);
        }

        void SecondaryWindow::setView(
            const tl::math::Vector2i& pos,
            double                    zoom,
            bool                      frame)
        {
            TLRENDER_P();
            p.viewport->setViewPosAndZoom(pos, zoom);
            p.viewport->setFrameView(frame);
        }
    }
}
