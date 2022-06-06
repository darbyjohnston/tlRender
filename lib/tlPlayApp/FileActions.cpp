// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FileActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/FilesModel.h>
#include <tlPlayApp/SettingsObject.h>

#include <QActionGroup>

namespace tl
{
    namespace play
    {
        struct FileActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QActionGroup* recentActionGroup = nullptr;
            QActionGroup* currentActionGroup = nullptr;

            QMenu* menu = nullptr;
            QMenu* recentMenu = nullptr;
            QMenu* currentMenu = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
        };

        FileActions::FileActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Open"] = new QAction(this);
            p.actions["Open"]->setText(tr("Open"));
            p.actions["Open"]->setIcon(QIcon(":/Icons/FileOpen.svg"));
            p.actions["Open"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
            p.actions["Open"]->setToolTip(tr("Open a file"));
            p.actions["OpenSeparateAudio"] = new QAction(this);
            p.actions["OpenSeparateAudio"]->setText(tr("Open With Separate Audio"));
            p.actions["OpenSeparateAudio"]->setIcon(QIcon(":/Icons/FileOpenSeparateAudio.svg"));
            p.actions["OpenSeparateAudio"]->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
            p.actions["OpenSeparateAudio"]->setToolTip(tr("Open a file with separate audio"));
            p.actions["Close"] = new QAction(this);
            p.actions["Close"]->setText(tr("Close"));
            p.actions["Close"]->setIcon(QIcon(":/Icons/FileClose.svg"));
            p.actions["Close"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
            p.actions["Close"]->setToolTip(tr("Close the current file"));
            p.actions["CloseAll"] = new QAction(this);
            p.actions["CloseAll"]->setText(tr("Close All"));
            p.actions["CloseAll"]->setIcon(QIcon(":/Icons/FileCloseAll.svg"));
            p.actions["CloseAll"]->setToolTip(tr("Close all files"));
            p.actions["Next"] = new QAction(this);
            p.actions["Next"]->setText(tr("Next"));
            p.actions["Next"]->setIcon(QIcon(":/Icons/Next.svg"));
            p.actions["Next"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
            p.actions["Next"]->setToolTip(tr("Change to the next file"));
            p.actions["Prev"] = new QAction(this);
            p.actions["Prev"]->setText(tr("Previous"));
            p.actions["Prev"]->setIcon(QIcon(":/Icons/Prev.svg"));
            p.actions["Prev"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
            p.actions["Prev"]->setToolTip(tr("Change to the previous file"));
            p.actions["NextLayer"] = new QAction(this);
            p.actions["NextLayer"]->setText(tr("Next Layer"));
            p.actions["NextLayer"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal));
            p.actions["NextLayer"]->setToolTip(tr("Change to the next layer"));
            p.actions["PrevLayer"] = new QAction(this);
            p.actions["PrevLayer"]->setText(tr("Previous Layer"));
            p.actions["PrevLayer"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
            p.actions["PrevLayer"]->setToolTip(tr("Change to the previous layer"));
            p.actions["Exit"] = new QAction(this);
            p.actions["Exit"]->setText(tr("Exit"));
            p.actions["Exit"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));

            p.recentActionGroup = new QActionGroup(this);

            p.currentActionGroup = new QActionGroup(this);
            p.currentActionGroup->setExclusive(true);

            p.menu = new QMenu;
            p.menu->setTitle(tr("&File"));
            p.menu->addAction(p.actions["Open"]);
            p.menu->addAction(p.actions["OpenSeparateAudio"]);
            p.menu->addAction(p.actions["Close"]);
            p.menu->addAction(p.actions["CloseAll"]);
            p.recentMenu = new QMenu;
            p.recentMenu->setTitle(tr("&Recent"));
            p.menu->addMenu(p.recentMenu);
            p.menu->addSeparator();
            p.currentMenu = new QMenu;
            p.currentMenu->setTitle(tr("&Current"));
            p.menu->addMenu(p.currentMenu);
            p.menu->addAction(p.actions["Next"]);
            p.menu->addAction(p.actions["Prev"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["NextLayer"]);
            p.menu->addAction(p.actions["PrevLayer"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Exit"]);

            _recentFilesUpdate();
            _actionsUpdate();

            connect(
                p.actions["Open"],
                &QAction::triggered,
                app,
                &App::openDialog);
            connect(
                p.actions["OpenSeparateAudio"],
                &QAction::triggered,
                app,
                &App::openSeparateAudioDialog);
            connect(
                p.actions["Close"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->close();
                });
            connect(
                p.actions["CloseAll"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->closeAll();
                });
            connect(
                p.actions["Next"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->next();
                });
            connect(
                p.actions["Prev"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prev();
                });
            connect(
                p.actions["NextLayer"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->nextLayer();
                });
            connect(
                p.actions["PrevLayer"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prevLayer();
                });
            connect(
                p.actions["Exit"],
                &QAction::triggered,
                app,
                &App::quit);

            connect(
                p.recentActionGroup,
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    app->open(action->data().toString());
                });

            connect(
                p.currentActionGroup,
                &QActionGroup::triggered,
                [this, app](QAction* action)
                {
                    const int index = _p->currentActionGroup->actions().indexOf(action);
                    app->filesModel()->setA(index);
                });

            connect(
                app->settingsObject(),
                SIGNAL(recentFilesChanged(const QList<QString>&)),
                SLOT(_recentFilesCallback()));

            p.filesObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >&)
                {
                    _actionsUpdate();
                });
            p.aIndexObserver = observer::ValueObserver<int>::create(
                app->filesModel()->observeAIndex(),
                [this](int)
                {
                    _actionsUpdate();
                });
        }

        FileActions::~FileActions()
        {}

        const QMap<QString, QAction*>& FileActions::actions() const
        {
            return _p->actions;
        }

        QMenu* FileActions::menu() const
        {
            return _p->menu;
        }

        void FileActions::_recentFilesCallback()
        {
            _recentFilesUpdate();
        }

        void FileActions::_recentFilesUpdate()
        {
            TLRENDER_P();
            for (const auto& i : p.recentActionGroup->actions())
            {
                p.recentActionGroup->removeAction(i);
                i->setParent(nullptr);
                delete i;
            }
            p.recentMenu->clear();
            const auto& recentFiles = p.app->settingsObject()->recentFiles();
            for (size_t i = 0; i < recentFiles.size(); ++i)
            {
                auto action = new QAction;
                const auto& file = recentFiles[i];
                action->setText(QString("%1 %2").arg(i + 1).arg(file));
                action->setData(file);
                p.recentActionGroup->addAction(action);
                p.recentMenu->addAction(action);
            }
        }

        void FileActions::_actionsUpdate()
        {
            TLRENDER_P();

            const auto& files = p.app->filesModel()->observeFiles()->get();
            const size_t count = files.size();
            p.actions["Close"]->setEnabled(count > 0);
            p.actions["CloseAll"]->setEnabled(count > 0);
            p.actions["Next"]->setEnabled(count > 1);
            p.actions["Prev"]->setEnabled(count > 1);
            p.actions["NextLayer"]->setEnabled(count > 0);
            p.actions["PrevLayer"]->setEnabled(count > 0);

            for (auto i : p.currentActionGroup->actions())
            {
                p.currentActionGroup->removeAction(i);
            }
            p.currentMenu->clear();
            const int aIndex = p.app->filesModel()->observeAIndex()->get();
            for (size_t i = 0; i < files.size(); ++i)
            {
                auto action = new QAction;
                action->setCheckable(true);
                action->setChecked(i == aIndex);
                action->setText(QString::fromUtf8(files[i]->path.get(-1, false).c_str()));
                p.currentActionGroup->addAction(action);
                p.currentMenu->addAction(action);
            }
        }
    }
}
