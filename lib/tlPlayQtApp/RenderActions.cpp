// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/RenderActions.h>

#include <tlPlayQtApp/App.h>

#include <tlQt/MetaTypes.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct RenderActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;
            QScopedPointer<QMenu> menu;

            std::shared_ptr<observer::ValueObserver<timeline::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
        };

        RenderActions::RenderActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Channels/Red"] = new QAction(this);
            p.actions["Channels/Red"]->setData(QVariant::fromValue<timeline::Channels>(timeline::Channels::Red));
            p.actions["Channels/Red"]->setCheckable(true);
            p.actions["Channels/Red"]->setText(tr("Red Channel"));
            p.actions["Channels/Red"]->setShortcut(QKeySequence(Qt::Key_R));
            p.actions["Channels/Green"] = new QAction(this);
            p.actions["Channels/Green"]->setData(QVariant::fromValue<timeline::Channels>(timeline::Channels::Green));
            p.actions["Channels/Green"]->setCheckable(true);
            p.actions["Channels/Green"]->setText(tr("Green Channel"));
            p.actions["Channels/Green"]->setShortcut(QKeySequence(Qt::Key_G));
            p.actions["Channels/Blue"] = new QAction(this);
            p.actions["Channels/Blue"]->setData(QVariant::fromValue<timeline::Channels>(timeline::Channels::Blue));
            p.actions["Channels/Blue"]->setCheckable(true);
            p.actions["Channels/Blue"]->setText(tr("Blue Channel"));
            p.actions["Channels/Blue"]->setShortcut(QKeySequence(Qt::Key_B));
            p.actions["Channels/Alpha"] = new QAction(this);
            p.actions["Channels/Alpha"]->setData(QVariant::fromValue<timeline::Channels>(timeline::Channels::Alpha));
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

            p.actions["VideoLevels/FromFile"] = new QAction(this);
            p.actions["VideoLevels/FromFile"]->setData(
                QVariant::fromValue<timeline::InputVideoLevels>(timeline::InputVideoLevels::FromFile));
            p.actions["VideoLevels/FromFile"]->setCheckable(true);
            p.actions["VideoLevels/FromFile"]->setText(tr("From File"));
            p.actions["VideoLevels/FullRange"] = new QAction(this);
            p.actions["VideoLevels/FullRange"]->setData(
                QVariant::fromValue<timeline::InputVideoLevels>(timeline::InputVideoLevels::FullRange));
            p.actions["VideoLevels/FullRange"]->setCheckable(true);
            p.actions["VideoLevels/FullRange"]->setText(tr("Full Range"));
            p.actions["VideoLevels/LegalRange"] = new QAction(this);
            p.actions["VideoLevels/LegalRange"]->setData(
                QVariant::fromValue<timeline::InputVideoLevels>(timeline::InputVideoLevels::LegalRange));
            p.actions["VideoLevels/LegalRange"]->setCheckable(true);
            p.actions["VideoLevels/LegalRange"]->setText(tr("Legal Range"));
            p.actionGroups["VideoLevels"] = new QActionGroup(this);
            p.actionGroups["VideoLevels"]->addAction(p.actions["VideoLevels/FromFile"]);
            p.actionGroups["VideoLevels"]->addAction(p.actions["VideoLevels/FullRange"]);
            p.actionGroups["VideoLevels"]->addAction(p.actions["VideoLevels/LegalRange"]);

            p.actions["AlphaBlend/None"] = new QAction(this);
            p.actions["AlphaBlend/None"]->setData(QVariant::fromValue<timeline::AlphaBlend>(timeline::AlphaBlend::None));
            p.actions["AlphaBlend/None"]->setCheckable(true);
            p.actions["AlphaBlend/None"]->setText(tr("None"));
            p.actions["AlphaBlend/Straight"] = new QAction(this);
            p.actions["AlphaBlend/Straight"]->setData(QVariant::fromValue<timeline::AlphaBlend>(timeline::AlphaBlend::Straight));
            p.actions["AlphaBlend/Straight"]->setCheckable(true);
            p.actions["AlphaBlend/Straight"]->setText(tr("Straight"));
            p.actions["AlphaBlend/Premultiplied"] = new QAction(this);
            p.actions["AlphaBlend/Premultiplied"]->setData(QVariant::fromValue<timeline::AlphaBlend>(timeline::AlphaBlend::Premultiplied));
            p.actions["AlphaBlend/Premultiplied"]->setCheckable(true);
            p.actions["AlphaBlend/Premultiplied"]->setText(tr("Premultiplied"));
            p.actionGroups["AlphaBlend"] = new QActionGroup(this);
            p.actionGroups["AlphaBlend"]->addAction(p.actions["AlphaBlend/None"]);
            p.actionGroups["AlphaBlend"]->addAction(p.actions["AlphaBlend/Straight"]);
            p.actionGroups["AlphaBlend"]->addAction(p.actions["AlphaBlend/Premultiplied"]);

            p.actions["MinifyFilter/Nearest"] = new QAction(this);
            p.actions["MinifyFilter/Nearest"]->setData(QVariant::fromValue<timeline::ImageFilter>(timeline::ImageFilter::Nearest));
            p.actions["MinifyFilter/Nearest"]->setCheckable(true);
            p.actions["MinifyFilter/Nearest"]->setText(tr("Nearest"));
            p.actions["MinifyFilter/Linear"] = new QAction(this);
            p.actions["MinifyFilter/Linear"]->setData(QVariant::fromValue<timeline::ImageFilter>(timeline::ImageFilter::Linear));
            p.actions["MinifyFilter/Linear"]->setCheckable(true);
            p.actions["MinifyFilter/Linear"]->setText(tr("Linear"));
            p.actionGroups["MinifyFilter"] = new QActionGroup(this);
            p.actionGroups["MinifyFilter"]->addAction(p.actions["MinifyFilter/Nearest"]);
            p.actionGroups["MinifyFilter"]->addAction(p.actions["MinifyFilter/Linear"]);

            p.actions["MagnifyFilter/Nearest"] = new QAction(this);
            p.actions["MagnifyFilter/Nearest"]->setData(QVariant::fromValue<timeline::ImageFilter>(timeline::ImageFilter::Nearest));
            p.actions["MagnifyFilter/Nearest"]->setCheckable(true);
            p.actions["MagnifyFilter/Nearest"]->setText(tr("Nearest"));
            p.actions["MagnifyFilter/Linear"] = new QAction(this);
            p.actions["MagnifyFilter/Linear"]->setData(QVariant::fromValue<timeline::ImageFilter>(timeline::ImageFilter::Linear));
            p.actions["MagnifyFilter/Linear"]->setCheckable(true);
            p.actions["MagnifyFilter/Linear"]->setText(tr("Linear"));
            p.actionGroups["MagnifyFilter"] = new QActionGroup(this);
            p.actionGroups["MagnifyFilter"]->addAction(p.actions["MagnifyFilter/Nearest"]);
            p.actionGroups["MagnifyFilter"]->addAction(p.actions["MagnifyFilter/Linear"]);

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Render"));
            p.menu->addAction(p.actions["Channels/Red"]);
            p.menu->addAction(p.actions["Channels/Green"]);
            p.menu->addAction(p.actions["Channels/Blue"]);
            p.menu->addAction(p.actions["Channels/Alpha"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["MirrorX"]);
            p.menu->addAction(p.actions["MirrorY"]);
            p.menu->addSeparator();
            auto videoLevelsMenu = p.menu->addMenu(tr("Video Levels"));
            videoLevelsMenu->addAction(p.actions["VideoLevels/FromFile"]);
            videoLevelsMenu->addAction(p.actions["VideoLevels/FullRange"]);
            videoLevelsMenu->addAction(p.actions["VideoLevels/LegalRange"]);
            auto alphaBlendMenu = p.menu->addMenu(tr("Alpha Blend"));
            alphaBlendMenu->addAction(p.actions["AlphaBlend/None"]);
            alphaBlendMenu->addAction(p.actions["AlphaBlend/Straight"]);
            alphaBlendMenu->addAction(p.actions["AlphaBlend/Premultiplied"]);
            auto minifyFilterMenu = p.menu->addMenu(tr("Minify Filter"));
            minifyFilterMenu->addAction(p.actions["MinifyFilter/Nearest"]);
            minifyFilterMenu->addAction(p.actions["MinifyFilter/Linear"]);
            auto magnifyFilterMenu = p.menu->addMenu(tr("Magnify Filter"));
            magnifyFilterMenu->addAction(p.actions["MagnifyFilter/Nearest"]);
            magnifyFilterMenu->addAction(p.actions["MagnifyFilter/Linear"]);

            _actionsUpdate();

            connect(
                p.actions["MirrorX"],
                &QAction::toggled,
                [app](bool value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.mirror.x = value;
                    app->colorModel()->setDisplayOptions(options);
                });
            connect(
                p.actions["MirrorY"],
                &QAction::toggled,
                [app](bool value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.mirror.y = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.actionGroups["Channels"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.channels = action->data().value<timeline::Channels>() != options.channels ?
                        action->data().value<timeline::Channels>() :
                        timeline::Channels::Color;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.actionGroups["VideoLevels"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->colorModel()->getImageOptions();
                    options.videoLevels = action->data().value<timeline::InputVideoLevels>();
                    app->colorModel()->setImageOptions(options);
                });

            connect(
                _p->actionGroups["AlphaBlend"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->colorModel()->getImageOptions();
                    options.alphaBlend = action->data().value<timeline::AlphaBlend>();
                    app->colorModel()->setImageOptions(options);
                });

            connect(
                _p->actionGroups["MinifyFilter"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.imageFilters.minify = action->data().value<timeline::ImageFilter>();
                    app->colorModel()->setDisplayOptions(options);
                });
            connect(
                _p->actionGroups["MagnifyFilter"],
                &QActionGroup::triggered,
                [app](QAction* action)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.imageFilters.magnify = action->data().value<timeline::ImageFilter>();
                    app->colorModel()->setDisplayOptions(options);
                });

            p.imageOptionsObserver = observer::ValueObserver<timeline::ImageOptions>::create(
                app->colorModel()->observeImageOptions(),
                [this](const timeline::ImageOptions&)
                {
                    _actionsUpdate();
                });

            p.displayOptionsObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->colorModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions&)
                {
                    _actionsUpdate();
                });
        }

        RenderActions::~RenderActions()
        {}

        const QMap<QString, QAction*>& RenderActions::actions() const
        {
            return _p->actions;
        }

        QMenu* RenderActions::menu() const
        {
            return _p->menu.get();
        }

        void RenderActions::_actionsUpdate()
        {
            TLRENDER_P();

            auto colorModel = p.app->colorModel();
            const auto& displayOptions = colorModel->getDisplayOptions();
            {
                QSignalBlocker blocker(p.actions["MirrorX"]);
                p.actions["MirrorX"]->setChecked(displayOptions.mirror.x);
            }
            {
                QSignalBlocker blocker(p.actions["MirrorY"]);
                p.actions["MirrorY"]->setChecked(displayOptions.mirror.y);
            }
            {
                QSignalBlocker blocker(p.actionGroups["Channels"]);
                p.actions["Channels/Red"]->setChecked(false);
                p.actions["Channels/Green"]->setChecked(false);
                p.actions["Channels/Blue"]->setChecked(false);
                p.actions["Channels/Alpha"]->setChecked(false);
                for (auto action : p.actionGroups["Channels"]->actions())
                {
                    if (action->data().value<timeline::Channels>() == displayOptions.channels)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            const auto& imageOptions = colorModel->getImageOptions();
            {
                QSignalBlocker blocker(p.actionGroups["VideoLevels"]);
                for (auto action : p.actionGroups["VideoLevels"]->actions())
                {
                    if (action->data().value<timeline::InputVideoLevels>() == imageOptions.videoLevels)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actionGroups["AlphaBlend"]);
                for (auto action : p.actionGroups["AlphaBlend"]->actions())
                {
                    if (action->data().value<timeline::AlphaBlend>() == imageOptions.alphaBlend)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
            {
                QSignalBlocker blocker(p.actionGroups["MinifyFilter"]);
                for (auto action : p.actionGroups["MinifyFilter"]->actions())
                {
                    if (action->data().value<timeline::ImageFilter>() == displayOptions.imageFilters.minify)
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
                    if (action->data().value<timeline::ImageFilter>() == displayOptions.imageFilters.magnify)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
        }
    }
}
