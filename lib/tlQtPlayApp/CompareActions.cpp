// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQtPlayApp/CompareActions.h>

#include <tlQtPlayApp/App.h>


#include <tlQt/MetaTypes.h>

#include <tlPlay/FilesModel.h>

#include <QActionGroup>

namespace tl
{
    namespace qtplay
    {
        struct CompareActions::Private
        {
            App* app = nullptr;

            timeline::CompareOptions compareOptions;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QMenu* menu = nullptr;
            QMenu* currentMenu = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
        };

        CompareActions::CompareActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["A"] = new QAction(this);
            p.actions["A"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::A));
            p.actions["A"]->setCheckable(true);
            p.actions["A"]->setText(tr("A"));
            p.actions["A"]->setIcon(QIcon(":/Icons/CompareA.svg"));
            p.actions["A"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
            p.actions["A"]->setToolTip(tr("Show the A file"));

            p.actions["B"] = new QAction(this);
            p.actions["B"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::B));
            p.actions["B"]->setCheckable(true);
            p.actions["B"]->setText(tr("B"));
            p.actions["B"]->setIcon(QIcon(":/Icons/CompareB.svg"));
            p.actions["B"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
            p.actions["B"]->setToolTip(tr("Show the B file"));

            p.actions["Wipe"] = new QAction(this);
            p.actions["Wipe"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::Wipe));
            p.actions["Wipe"]->setCheckable(true);
            p.actions["Wipe"]->setText(tr("Wipe"));
            p.actions["Wipe"]->setIcon(QIcon(":/Icons/CompareWipe.svg"));
            p.actions["Wipe"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
            p.actions["Wipe"]->setToolTip(tr(
                "Wipe between the A and B files\n\n"
                "Use the Alt key + left mouse button to move the wipe"));

            p.actions["Overlay"] = new QAction(this);
            p.actions["Overlay"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::Overlay));
            p.actions["Overlay"]->setCheckable(true);
            p.actions["Overlay"]->setText(tr("Overlay"));
            p.actions["Overlay"]->setIcon(QIcon(":/Icons/CompareOverlay.svg"));
            p.actions["Overlay"]->setToolTip(tr("Overlay the A and B files with transparency"));

            p.actions["Difference"] = new QAction(this);
            p.actions["Difference"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::Difference));
            p.actions["Difference"]->setCheckable(true);
            p.actions["Difference"]->setText(tr("Difference"));
            p.actions["Difference"]->setIcon(QIcon(":/Icons/CompareDifference.svg"));
            p.actions["Difference"]->setToolTip(tr("Difference the A and B files"));

            p.actions["Horizontal"] = new QAction(this);
            p.actions["Horizontal"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::Horizontal));
            p.actions["Horizontal"]->setCheckable(true);
            p.actions["Horizontal"]->setText(tr("Horizontal"));
            p.actions["Horizontal"]->setIcon(QIcon(":/Icons/CompareHorizontal.svg"));
            p.actions["Horizontal"]->setToolTip(tr("Show the A and B files side by side"));

            p.actions["Vertical"] = new QAction(this);
            p.actions["Vertical"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::Vertical));
            p.actions["Vertical"]->setCheckable(true);
            p.actions["Vertical"]->setText(tr("Vertical"));
            p.actions["Vertical"]->setIcon(QIcon(":/Icons/CompareVertical.svg"));
            p.actions["Vertical"]->setToolTip(tr("Show the A file above the B file"));

            p.actions["Tile"] = new QAction(this);
            p.actions["Tile"]->setData(QVariant::fromValue<timeline::CompareMode>(timeline::CompareMode::Tile));
            p.actions["Tile"]->setCheckable(true);
            p.actions["Tile"]->setText(tr("Tile"));
            p.actions["Tile"]->setIcon(QIcon(":/Icons/CompareTile.svg"));
            p.actions["Tile"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
            p.actions["Tile"]->setToolTip(tr("Tile the A and B files"));

            p.actions["Next"] = new QAction(this);
            p.actions["Next"]->setText(tr("Next"));
            p.actions["Next"]->setIcon(QIcon(":/Icons/Next.svg"));
            p.actions["Next"]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_PageDown));
            p.actions["Next"]->setToolTip(tr("Change to the next file"));

            p.actions["Prev"] = new QAction(this);
            p.actions["Prev"]->setText(tr("Previous"));
            p.actions["Prev"]->setIcon(QIcon(":/Icons/Prev.svg"));
            p.actions["Prev"]->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_PageUp));
            p.actions["Prev"]->setToolTip(tr("Change to the previous file"));

            p.actionGroups["Current"] = new QActionGroup(this);

            p.actionGroups["Compare"] = new QActionGroup(this);
            p.actionGroups["Compare"]->setExclusive(true);
            p.actionGroups["Compare"]->addAction(p.actions["A"]);
            p.actionGroups["Compare"]->addAction(p.actions["B"]);
            p.actionGroups["Compare"]->addAction(p.actions["Wipe"]);
            p.actionGroups["Compare"]->addAction(p.actions["Overlay"]);
            p.actionGroups["Compare"]->addAction(p.actions["Difference"]);
            p.actionGroups["Compare"]->addAction(p.actions["Horizontal"]);
            p.actionGroups["Compare"]->addAction(p.actions["Vertical"]);
            p.actionGroups["Compare"]->addAction(p.actions["Tile"]);

            p.menu = new QMenu;
            p.menu->setTitle(tr("&Compare"));
            p.currentMenu = new QMenu;
            p.currentMenu->setTitle(tr("&Current"));
            p.menu->addMenu(p.currentMenu);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["A"]);
            p.menu->addAction(p.actions["B"]);
            p.menu->addAction(p.actions["Wipe"]);
            p.menu->addAction(p.actions["Overlay"]);
            p.menu->addAction(p.actions["Difference"]);
            p.menu->addAction(p.actions["Horizontal"]);
            p.menu->addAction(p.actions["Vertical"]);
            p.menu->addAction(p.actions["Tile"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Next"]);
            p.menu->addAction(p.actions["Prev"]);

            _actionsUpdate();

            connect(
                p.actions["Next"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->nextB();
                });
            connect(
                p.actions["Prev"],
                &QAction::triggered,
                [app]
                {
                    app->filesModel()->prevB();
                });

            connect(
                p.actionGroups["Current"],
                &QActionGroup::triggered,
                [this, app](QAction* action)
                {
                    const int index = _p->actionGroups["Current"]->actions().indexOf(action);
                    const auto bIndexes = app->filesModel()->observeBIndexes()->get();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index);
                    app->filesModel()->setB(index, i == bIndexes.end());
                });

            connect(
                p.actionGroups["Compare"],
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    auto compareOptions = _p->compareOptions;
                    compareOptions.mode = action->data().value<timeline::CompareMode>();
                    _p->app->filesModel()->setCompareOptions(compareOptions);
                });

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >&)
                {
                    _actionsUpdate();
                });
            p.bIndexesObserver = observer::ListObserver<int>::create(
                app->filesModel()->observeBIndexes(),
                [this](const std::vector<int>&)
                {
                    _actionsUpdate();
                });
        }

        CompareActions::~CompareActions()
        {}

        const QMap<QString, QAction*>& CompareActions::actions() const
        {
            return _p->actions;
        }

        QMenu* CompareActions::menu() const
        {
            return _p->menu;
        }

        void CompareActions::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            _actionsUpdate();
        }

        void CompareActions::_actionsUpdate()
        {
            TLRENDER_P();

            const auto& files = p.app->filesModel()->observeFiles()->get();
            const size_t count = files.size();
            for (auto i : p.actions)
            {
                i->setEnabled(count > 0);
            }

            for (auto i : p.actionGroups["Current"]->actions())
            {
                p.actionGroups["Current"]->removeAction(i);
            }
            p.currentMenu->clear();
            const auto bIndexes = p.app->filesModel()->observeBIndexes()->get();
            for (size_t i = 0; i < count; ++i)
            {
                auto action = new QAction;
                action->setCheckable(true);
                action->setChecked(std::find(bIndexes.begin(), bIndexes.end(), i) != bIndexes.end());
                action->setText(QString::fromUtf8(files[i]->path.get(-1, false).c_str()));
                p.actionGroups["Current"]->addAction(action);
                p.currentMenu->addAction(action);
            }

            if (count > 0)
            {
                QSignalBlocker blocker(p.actionGroups["Compare"]);
                for (auto action : p.actionGroups["Compare"]->actions())
                {
                    if (action->data().value<timeline::CompareMode>() == p.compareOptions.mode)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            else
            {
                QSignalBlocker blocker(p.actionGroups["Compare"]);
                p.actions["A"]->setChecked(true);
            }
        }
    }
}
