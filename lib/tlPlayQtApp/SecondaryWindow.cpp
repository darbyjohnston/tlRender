// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SecondaryWindow.h>

#include <tlPlayQtApp/App.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/RenderModel.h>
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

            QMetaObject::Connection playerChangedConnection;

            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > ocioOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > backgroundOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<dtk::ImageType> > colorBufferObserver;
        };

        SecondaryWindow::SecondaryWindow(
            App* app,
            QWidget* parent) :
            QWidget(parent),
            _p(new  Private)
        {
            DTK_P();

            setAttribute(Qt::WA_DeleteOnClose);

            p.app = app;

            p.viewport = new qtwidget::TimelineViewport(
                app->getContext(),
                dtk::Style::create(app->getContext()));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.viewport);
            setLayout(layout);

            resize(1920, 1080);

            p.viewport->setPlayer(app->player());

            p.playerChangedConnection = connect(
                app,
                &App::playerChanged,
                [this](const QSharedPointer<qt::TimelinePlayer>& value)
                {
                    _p->viewport->setPlayer(value);
                });

            p.compareOptionsObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _p->viewport->setCompareOptions(value);
                });

            p.ocioOptionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                app->colorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                    _p->viewport->setOCIOOptions(value);
                });

            p.lutOptionsObserver = dtk::ValueObserver<timeline::LUTOptions>::create(
                app->colorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _p->viewport->setLUTOptions(value);
                });

            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->viewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->viewport->setDisplayOptions({ value });
                });

            p.backgroundOptionsObserver = dtk::ValueObserver<timeline::BackgroundOptions>::create(
                app->viewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _p->viewport->setBackgroundOptions(value);
                });

            p.imageOptionsObserver = dtk::ValueObserver<dtk::ImageOptions>::create(
                app->renderModel()->observeImageOptions(),
                [this](const dtk::ImageOptions& value)
                {
                    _p->viewport->setImageOptions({ value });
                });

            p.colorBufferObserver = dtk::ValueObserver<dtk::ImageType>::create(
                app->renderModel()->observeColorBuffer(),
                [this](dtk::ImageType value)
                {
                    _p->viewport->setColorBuffer(value);
                });
        }

        SecondaryWindow::~SecondaryWindow()
        {
            //! \bug Why is is neccesary to manually disconnect here?
            //! Without the disconnect the signal is still received
            //! after the object is destroyed.
            if (_p->playerChangedConnection)
            {
                disconnect(_p->playerChangedConnection);
            }
        }

        void SecondaryWindow::setView(
            const dtk::V2I& pos,
            double          zoom,
            bool            frame)
        {
            DTK_P();
            p.viewport->setViewPosAndZoom(pos, zoom);
            p.viewport->setFrameView(frame);
        }
    }
}
