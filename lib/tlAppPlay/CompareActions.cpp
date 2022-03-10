// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppPlay/CompareActions.h>

#include <tlAppPlay/App.h>
#include <tlAppPlay/FilesModel.h>

#include <tlQt/MetaTypes.h>

#include <QActionGroup>

namespace tl
{
    namespace play
    {
        struct CompareActions::Private
        {
            App* app = nullptr;

            timeline::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;

            QMap<QString, QAction*> actions;
            QActionGroup* compareActionGroup = nullptr;

            QMenu* menu = nullptr;
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
            p.actions["Overlay"]->setToolTip(tr("Overlay the A file over the B file with transparency"));
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
            p.compareActionGroup = new QActionGroup(this);
            p.compareActionGroup->setExclusive(true);
            p.compareActionGroup->addAction(p.actions["A"]);
            p.compareActionGroup->addAction(p.actions["B"]);
            p.compareActionGroup->addAction(p.actions["Wipe"]);
            p.compareActionGroup->addAction(p.actions["Overlay"]);
            p.compareActionGroup->addAction(p.actions["Horizontal"]);
            p.compareActionGroup->addAction(p.actions["Vertical"]);
            p.compareActionGroup->addAction(p.actions["Tile"]);
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

            p.menu = new QMenu;
            p.menu->setTitle(tr("&Compare"));
            p.menu->addAction(p.actions["A"]);
            p.menu->addAction(p.actions["B"]);
            p.menu->addAction(p.actions["Wipe"]);
            p.menu->addAction(p.actions["Overlay"]);
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
                p.compareActionGroup,
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    auto compareOptions = _p->compareOptions;
                    compareOptions.mode = action->data().value<timeline::CompareMode>();
                    _p->app->filesModel()->setCompareOptions(compareOptions);
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

        void CompareActions::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
        {
            TLRENDER_P();
            p.timelinePlayers = timelinePlayers;
            _actionsUpdate();
        }

        void CompareActions::_actionsUpdate()
        {
            TLRENDER_P();

            const int count = p.timelinePlayers.size();
            p.actions["A"]->setEnabled(count > 0);
            p.actions["B"]->setEnabled(count > 0);
            p.actions["Wipe"]->setEnabled(count > 0);
            p.actions["Overlay"]->setEnabled(count > 0);
            p.actions["Horizontal"]->setEnabled(count > 0);
            p.actions["Vertical"]->setEnabled(count > 0);
            p.actions["Tile"]->setEnabled(count > 0);
            p.actions["Next"]->setEnabled(count > 0);
            p.actions["Prev"]->setEnabled(count > 0);

            if (!p.timelinePlayers.empty())
            {
                {
                    QSignalBlocker blocker(p.compareActionGroup);
                    for (auto action : p.compareActionGroup->actions())
                    {
                        if (action->data().value<timeline::CompareMode>() == p.compareOptions.mode)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(p.compareActionGroup);
                    p.actions["A"]->setChecked(true);
                }
            }
        }
    }
}
