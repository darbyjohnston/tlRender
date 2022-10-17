// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/MainWindow.h>

#include <tlViewApp/App.h>
#include <tlViewApp/SceneView.h>
#include <tlViewApp/TimelineScene.h>

#include <QDragEnterEvent>
#include <QMenuBar>
#include <QMimeData>
#include <QStatusBar>

namespace tl
{
    namespace view
    {
        namespace
        {
            const size_t errorTimeout = 5000;
        }

        struct MainWindow::Private
        {
            App* app = nullptr;

            otio::SerializableObject::Retainer<otio::Timeline> timeline;
            std::shared_ptr<TimelineItem> timelineItem = nullptr;
            SceneView* sceneView = nullptr;
            QStatusBar* statusBar = nullptr;

            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        MainWindow::MainWindow(App* app, QWidget* parent) :
            QMainWindow(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            setFocusPolicy(Qt::ClickFocus);
            setAcceptDrops(true);

            auto menuBar = new QMenuBar;
            setMenuBar(menuBar);

            p.sceneView = new SceneView(app->getContext());
            setCentralWidget(p.sceneView);

            p.statusBar = new QStatusBar;
            setStatusBar(p.statusBar);

            p.sceneView->setFocus();

            _sceneUpdate();

            resize(800, 600);

            connect(
                app,
                &App::timelineChanged,
                [this](otio::Timeline* timeline)
                {
                    _p->timeline = timeline;
                    _sceneUpdate();
                });

            p.logObserver = observer::ListObserver<log::Item>::create(
                app->getContext()->getLogSystem()->observeLog(),
                [this](const std::vector<log::Item>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case log::Type::Error:
                            _p->statusBar->showMessage(
                                QString(tr("ERROR: %1")).
                                arg(QString::fromUtf8(i.message.c_str())),
                                errorTimeout);
                            break;
                        default: break;
                        }
                    }
                });
        }

        MainWindow::~MainWindow()
        {
            TLRENDER_P();
        }

        void MainWindow::closeEvent(QCloseEvent*)
        {
            TLRENDER_P();
        }

        void MainWindow::dragEnterEvent(QDragEnterEvent* event)
        {
            const QMimeData* mimeData = event->mimeData();
            if (mimeData->hasUrls())
            {
                event->acceptProposedAction();
            }
        }

        void MainWindow::dragMoveEvent(QDragMoveEvent* event)
        {
            const QMimeData* mimeData = event->mimeData();
            if (mimeData->hasUrls())
            {
                event->acceptProposedAction();
            }
        }

        void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
        {
            event->accept();
        }

        void MainWindow::dropEvent(QDropEvent* event)
        {
            TLRENDER_P();
            const QMimeData* mimeData = event->mimeData();
            if (mimeData->hasUrls())
            {
                const auto urlList = mimeData->urls();
                for (int i = 0; i < urlList.size(); ++i)
                {
                    const QString fileName = urlList[i].toLocalFile();
                    p.app->open(fileName);
                }
            }
        }

        void MainWindow::_sceneUpdate()
        {
            TLRENDER_P();
            if (p.timelineItem)
            {
                p.sceneView->setScene(nullptr);
                p.timelineItem.reset();
            }
            if (p.timeline)
            {
                p.timelineItem = createScene(p.timeline);
                p.sceneView->setScene(p.timelineItem);
            }
        }
    }
}
