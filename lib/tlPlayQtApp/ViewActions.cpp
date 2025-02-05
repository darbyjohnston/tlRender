// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ViewActions.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/MainWindow.h>

#include <tlPlay/Viewport.h>
#include <tlPlay/ViewportModel.h>

#include <tlQt/MetaTypes.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct ViewActions::Private
        {
            App* app = nullptr;
            MainWindow* mainWindow = nullptr;
            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;
            QScopedPointer<QMenu> menu;
            std::shared_ptr<dtk::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > hudObserver;
        };

        ViewActions::ViewActions(App* app, MainWindow* mainWindow, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;
            p.mainWindow = mainWindow;

            p.actions["Frame"] = new QAction(this);
            p.actions["Frame"]->setCheckable(true);
            p.actions["Frame"]->setText(tr("Frame"));
            p.actions["Frame"]->setIcon(QIcon(":/Icons/ViewFrame.svg"));
            p.actions["Frame"]->setToolTip(tr("Frame the view to fit the window"));

            p.actions["Zoom1To1"] = new QAction(this);
            p.actions["Zoom1To1"]->setText(tr("Zoom 1:1"));
            p.actions["Zoom1To1"]->setIcon(QIcon(":/Icons/ViewZoom1To1.svg"));
            p.actions["Zoom1To1"]->setToolTip(tr("Set the view zoom to 1:1"));

            p.actions["ZoomIn"] = new QAction(this);
            p.actions["ZoomIn"]->setText(tr("Zoom In"));

            p.actions["ZoomOut"] = new QAction(this);
            p.actions["ZoomOut"]->setText(tr("Zoom Out"));

            p.actions["Channels/Red"] = new QAction(this);
            p.actions["Channels/Red"]->setData(QVariant::fromValue<dtk::ChannelDisplay>(dtk::ChannelDisplay::Red));
            p.actions["Channels/Red"]->setCheckable(true);
            p.actions["Channels/Red"]->setText(tr("Red Channel"));
            p.actions["Channels/Red"]->setShortcut(QKeySequence(Qt::Key_R));
            p.actions["Channels/Green"] = new QAction(this);
            p.actions["Channels/Green"]->setData(QVariant::fromValue<dtk::ChannelDisplay>(dtk::ChannelDisplay::Green));
            p.actions["Channels/Green"]->setCheckable(true);
            p.actions["Channels/Green"]->setText(tr("Green Channel"));
            p.actions["Channels/Green"]->setShortcut(QKeySequence(Qt::Key_G));
            p.actions["Channels/Blue"] = new QAction(this);
            p.actions["Channels/Blue"]->setData(QVariant::fromValue<dtk::ChannelDisplay>(dtk::ChannelDisplay::Blue));
            p.actions["Channels/Blue"]->setCheckable(true);
            p.actions["Channels/Blue"]->setText(tr("Blue Channel"));
            p.actions["Channels/Blue"]->setShortcut(QKeySequence(Qt::Key_B));
            p.actions["Channels/Alpha"] = new QAction(this);
            p.actions["Channels/Alpha"]->setData(QVariant::fromValue<dtk::ChannelDisplay>(dtk::ChannelDisplay::Alpha));
            p.actions["Channels/Alpha"]->setCheckable(true);
            p.actions["Channels/Alpha"]->setText(tr("Alpha Channel"));
            p.actions["Channels/Alpha"]->setShortcut(QKeySequence(Qt::Key_A));
            p.actionGroups["Channels"] = new QActionGroup(this);
            p.actionGroups["Channels"]->addAction(p.actions["Channels/Red"]);
            p.actionGroups["Channels"]->addAction(p.actions["Channels/Green"]);
            p.actionGroups["Channels"]->addAction(p.actions["Channels/Blue"]);
            p.actionGroups["Channels"]->addAction(p.actions["Channels/Alpha"]);

            p.actions["MirrorX"] = new QAction(this);
            p.actions["MirrorX"]->setText(tr("Mirror Horizontal"));
            p.actions["MirrorX"]->setShortcut(QKeySequence(Qt::Key_H));
            p.actions["MirrorX"]->setCheckable(true);
            p.actions["MirrorY"] = new QAction(this);
            p.actions["MirrorY"]->setText(tr("Mirror Vertical"));
            p.actions["MirrorY"]->setShortcut(QKeySequence(Qt::Key_V));
            p.actions["MirrorY"]->setCheckable(true);

            p.actions["MinifyFilter/Nearest"] = new QAction(this);
            p.actions["MinifyFilter/Nearest"]->setData(QVariant::fromValue<dtk::ImageFilter>(dtk::ImageFilter::Nearest));
            p.actions["MinifyFilter/Nearest"]->setCheckable(true);
            p.actions["MinifyFilter/Nearest"]->setText(tr("Nearest"));
            p.actions["MinifyFilter/Linear"] = new QAction(this);
            p.actions["MinifyFilter/Linear"]->setData(QVariant::fromValue<dtk::ImageFilter>(dtk::ImageFilter::Linear));
            p.actions["MinifyFilter/Linear"]->setCheckable(true);
            p.actions["MinifyFilter/Linear"]->setText(tr("Linear"));
            p.actionGroups["MinifyFilter"] = new QActionGroup(this);
            p.actionGroups["MinifyFilter"]->addAction(p.actions["MinifyFilter/Nearest"]);
            p.actionGroups["MinifyFilter"]->addAction(p.actions["MinifyFilter/Linear"]);

            p.actions["MagnifyFilter/Nearest"] = new QAction(this);
            p.actions["MagnifyFilter/Nearest"]->setData(QVariant::fromValue<dtk::ImageFilter>(dtk::ImageFilter::Nearest));
            p.actions["MagnifyFilter/Nearest"]->setCheckable(true);
            p.actions["MagnifyFilter/Nearest"]->setText(tr("Nearest"));
            p.actions["MagnifyFilter/Linear"] = new QAction(this);
            p.actions["MagnifyFilter/Linear"]->setData(QVariant::fromValue<dtk::ImageFilter>(dtk::ImageFilter::Linear));
            p.actions["MagnifyFilter/Linear"]->setCheckable(true);
            p.actions["MagnifyFilter/Linear"]->setText(tr("Linear"));
            p.actionGroups["MagnifyFilter"] = new QActionGroup(this);
            p.actionGroups["MagnifyFilter"]->addAction(p.actions["MagnifyFilter/Nearest"]);
            p.actionGroups["MagnifyFilter"]->addAction(p.actions["MagnifyFilter/Linear"]);

            p.actions["HUD"] = new QAction(this);
            p.actions["HUD"]->setCheckable(true);
            p.actions["HUD"]->setText(tr("HUD"));
            p.actions["HUD"]->setShortcut(QKeySequence(QKeySequence(Qt::CTRL | Qt::Key_H)));

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&View"));
            p.menu->addAction(p.actions["Frame"]);
            p.menu->addAction(p.actions["Zoom1To1"]);
            p.menu->addAction(p.actions["ZoomIn"]);
            p.menu->addAction(p.actions["ZoomOut"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Channels/Red"]);
            p.menu->addAction(p.actions["Channels/Green"]);
            p.menu->addAction(p.actions["Channels/Blue"]);
            p.menu->addAction(p.actions["Channels/Alpha"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["MirrorX"]);
            p.menu->addAction(p.actions["MirrorY"]);
            auto minifyFilterMenu = p.menu->addMenu(tr("Minify Filter"));
            minifyFilterMenu->addAction(p.actions["MinifyFilter/Nearest"]);
            minifyFilterMenu->addAction(p.actions["MinifyFilter/Linear"]);
            auto magnifyFilterMenu = p.menu->addMenu(tr("Magnify Filter"));
            magnifyFilterMenu->addAction(p.actions["MagnifyFilter/Nearest"]);
            magnifyFilterMenu->addAction(p.actions["MagnifyFilter/Linear"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["HUD"]);

            _actionsUpdate();

            connect(
                p.actionGroups["Channels"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.channels = action->data().value<dtk::ChannelDisplay>() != options.channels ?
                        action->data().value<dtk::ChannelDisplay>() :
                        dtk::ChannelDisplay::Color;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.actions["MirrorX"],
                &QAction::toggled,
                [app](bool value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.mirror.x = value;
                    app->viewportModel()->setDisplayOptions(options);
                });
            connect(
                p.actions["MirrorY"],
                &QAction::toggled,
                [app](bool value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.mirror.y = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                _p->actionGroups["MinifyFilter"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.imageFilters.minify = action->data().value<dtk::ImageFilter>();
                    app->viewportModel()->setDisplayOptions(options);
                });
            connect(
                _p->actionGroups["MagnifyFilter"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.imageFilters.magnify = action->data().value<dtk::ImageFilter>();
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.actions["HUD"],
                &QAction::toggled,
                [mainWindow](bool value)
                {
                    mainWindow->viewport()->setHUD(value);
                });

            p.frameViewObserver = dtk::ValueObserver<bool>::create(
                mainWindow->viewport()->observeFrameView(),
                [this](bool)
                {
                    _actionsUpdate();
                });

            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->viewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions&)
                {
                    _actionsUpdate();
                });

            p.hudObserver = dtk::ValueObserver<bool>::create(
                mainWindow->viewport()->observeHUD(),
                [this](bool)
                {
                    _actionsUpdate();
                });
        }

        ViewActions::~ViewActions()
        {}

        const QMap<QString, QAction*>& ViewActions::actions() const
        {
            return _p->actions;
        }

        QMenu* ViewActions::menu() const
        {
            return _p->menu.get();
        }

        void ViewActions::_actionsUpdate()
        {
            TLRENDER_P();
            p.actions["Frame"]->setChecked(p.mainWindow->viewport()->hasFrameView());

            auto viewportModel = p.app->viewportModel();
            const auto& displayOptions = viewportModel->getDisplayOptions();
            {
                QSignalBlocker blocker(p.actionGroups["Channels"]);
                p.actions["Channels/Red"]->setChecked(false);
                p.actions["Channels/Green"]->setChecked(false);
                p.actions["Channels/Blue"]->setChecked(false);
                p.actions["Channels/Alpha"]->setChecked(false);
                for (auto action : p.actionGroups["Channels"]->actions())
                {
                    if (action->data().value<dtk::ChannelDisplay>() == displayOptions.channels)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actions["MirrorX"]);
                p.actions["MirrorX"]->setChecked(displayOptions.mirror.x);
            }
            {
                QSignalBlocker blocker(p.actions["MirrorY"]);
                p.actions["MirrorY"]->setChecked(displayOptions.mirror.y);
            }
            {
                QSignalBlocker blocker(p.actionGroups["MinifyFilter"]);
                for (auto action : p.actionGroups["MinifyFilter"]->actions())
                {
                    if (action->data().value<dtk::ImageFilter>() == displayOptions.imageFilters.minify)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actionGroups["MagnifyFilter"]);
                for (auto action : p.actionGroups["MagnifyFilter"]->actions())
                {
                    if (action->data().value<dtk::ImageFilter>() == displayOptions.imageFilters.magnify)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actions["HUD"]);
                p.actions["HUD"]->setChecked(p.mainWindow->viewport()->hasHUD());
            }
        }
    }
}
