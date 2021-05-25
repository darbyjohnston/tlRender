// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"

#include <tlrCore/File.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFileDialog>
#include <QMenuBar>
#include <QMimeData>
#include <QSettings>

namespace tlr
{
    MainWindow::MainWindow(
        SettingsObject* settingsObject,
        qt::TimeObject* timeObject,
        QWidget* parent) :
        QMainWindow(parent),
        _settingsObject(settingsObject),
        _timeObject(timeObject)
    {
        setFocusPolicy(Qt::ClickFocus);
        setAcceptDrops(true);

        _actions["File/Open"] = new QAction(this);
        _actions["File/Open"]->setText(tr("Open"));
        _actions["File/Open"]->setShortcut(QKeySequence::Open);
        _actions["File/CloseAll"] = new QAction(this);
        _actions["File/CloseAll"]->setText(tr("Close All"));
        _recentFilesActionGroup = new QActionGroup(this);
        _actions["File/Exit"] = new QAction(this);
        _actions["File/Exit"]->setText(tr("Exit"));
        _actions["File/Exit"]->setShortcut(QKeySequence::Quit);

        auto fileMenu = new QMenu;
        fileMenu->setTitle(tr("&File"));
        fileMenu->addAction(_actions["File/Open"]);
        fileMenu->addAction(_actions["File/CloseAll"]);
        fileMenu->addSeparator();
        _recentFilesMenu = new QMenu;
        _recentFilesMenu->setTitle(tr("&Recent Files"));
        fileMenu->addMenu(_recentFilesMenu);
        fileMenu->addSeparator();
        fileMenu->addAction(_actions["File/Exit"]);

        auto menuBar = new QMenuBar;
        menuBar->addMenu(fileMenu);
        setMenuBar(menuBar);

        _scrollLayout = new QVBoxLayout;
        auto scrollWidget = new QWidget;
        scrollWidget->setLayout(_scrollLayout);
        _scrollArea = new QScrollArea;
        _scrollArea->setWidget(scrollWidget);
        _scrollArea->setWidgetResizable(true);
        setCentralWidget(_scrollArea);

        _recentFilesUpdate();

        connect(
            _actions["File/Open"],
            SIGNAL(triggered()),
            SLOT(_openCallback()));
        connect(
            _actions["File/CloseAll"],
            SIGNAL(triggered()),
            SLOT(_closeAllCallback()));
        connect(
            _recentFilesActionGroup,
            SIGNAL(triggered(QAction*)),
            SLOT(_recentFilesCallback(QAction*)));
        connect(
            _actions["File/Exit"],
            SIGNAL(triggered()),
            qApp,
            SLOT(quit()));

        connect(
            _settingsObject,
            SIGNAL(recentFilesChanged(const QList<QString>&)),
            SLOT(_recentFilesCallback()));

        if (auto app = qobject_cast<App*>(qApp))
        {
            connect(
                app,
                SIGNAL(opened(const std::shared_ptr<tlr::timeline::Timeline>&)),
                SLOT(_openedCallback(const std::shared_ptr<tlr::timeline::Timeline>&)));
            connect(
                app,
                SIGNAL(closed(const std::shared_ptr<tlr::timeline::Timeline>&)),
                SLOT(_closedCallback(const std::shared_ptr<tlr::timeline::Timeline>&)));
            connect(
                app,
                SIGNAL(aboutToQuit()),
                SLOT(_saveSettingsCallback()));
        }

        resize(640, 360);
        QSettings settings;
        auto ba = settings.value("geometry").toByteArray();
        if (!ba.isEmpty())
        {
            restoreGeometry(settings.value("geometry").toByteArray());
        }
        ba = settings.value("geometry").toByteArray();
        if (!ba.isEmpty())
        {
            restoreState(settings.value("windowState").toByteArray());
        }
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        _saveSettingsCallback();
        QMainWindow::closeEvent(event);
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
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls())
        {
            const auto urlList = mimeData->urls();
            for (int i = 0; i < urlList.size(); ++i)
            {
                if (auto app = qobject_cast<App*>(qApp))
                {
                    app->open(urlList[i].toLocalFile());
                }
            }
        }
    }

    void MainWindow::_openCallback()
    {
        std::vector<std::string> extensions;
        for (const auto& i : timeline::getExtensions())
        {
            extensions.push_back("*" + i);
        }

        QString dir;

        const auto fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open Timeline"),
            dir,
            tr("Timeline Files") + " (" + string::join(extensions, ", ").c_str() + ")");
        if (!fileName.isEmpty())
        {
            if (auto app = qobject_cast<App*>(qApp))
            {
                app->open(fileName);
            }
        }
    }

    void MainWindow::_openedCallback(const std::shared_ptr<timeline::Timeline>& timeline)
    {
        _timelines.append(timeline);
        auto widget = new qt::FilmstripWidget;
        widget->setTimeline(timeline);
        widget->setToolTip(timeline->getFileName().c_str());
        _filmstripWidgets[timeline] = widget;
        _scrollLayout->addWidget(widget);
    }

    void MainWindow::_closeAllCallback()
    {
        if (auto app = qobject_cast<App*>(qApp))
        {
            app->closeAll();
        }
    }

    void MainWindow::_closedCallback(const std::shared_ptr<timeline::Timeline>& timeline)
    {
        int i = _timelines.indexOf(timeline);
        if (i != -1)
        {
            const auto j = _filmstripWidgets.find(timeline);
            if (j != _filmstripWidgets.end())
            {
                j.value()->setParent(nullptr);
                delete j.value();
                _filmstripWidgets.erase(j);
            }

            _timelines.removeOne(timeline);
        }
    }

    void MainWindow::_recentFilesCallback(QAction* action)
    {
        const auto i = _actionToRecentFile.find(action);
        if (i != _actionToRecentFile.end())
        {
            if (auto app = qobject_cast<App*>(qApp))
            {
                app->open(i.value());
            }
        }
    }

    void MainWindow::_recentFilesCallback()
    {
        _recentFilesUpdate();
    }

    void MainWindow::_saveSettingsCallback()
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
    }

    void MainWindow::_recentFilesUpdate()
    {
        for (const auto& i : _actionToRecentFile.keys())
        {
            _recentFilesActionGroup->removeAction(i);
            i->setParent(nullptr);
            delete i;
        }
        _actionToRecentFile.clear();
        _recentFilesMenu->clear();
        const auto& recentFiles = _settingsObject->recentFiles();
        for (size_t i = 0; i < recentFiles.size(); ++i)
        {
            auto action = new QAction;
            const auto& file = recentFiles[i];
            action->setText(QString("%1 %2").arg(i + 1).arg(file));
            _recentFilesActionGroup->addAction(action);
            _actionToRecentFile[action] = file;
            _recentFilesMenu->addAction(action);
        }
    }
}
