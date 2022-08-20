// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ImageActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/FilesModel.h>

#include <tlQt/MetaTypes.h>

#include <QActionGroup>

namespace tl
{
    namespace play
    {
        struct ImageActions::Private
        {
            App* app = nullptr;

            timeline::ImageOptions imageOptions;
            timeline::DisplayOptions displayOptions;

            QMap<QString, QAction*> actions;
            QActionGroup* videoLevelsActionGroup = nullptr;
            QActionGroup* channelsActionGroup = nullptr;
            QActionGroup* alphaBlendActionGroup = nullptr;

            QMenu* menu = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
        };

        ImageActions::ImageActions(App* app, QObject* parent) :
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
            p.channelsActionGroup = new QActionGroup(this);
            p.channelsActionGroup->addAction(p.actions["Channels/Red"]);
            p.channelsActionGroup->addAction(p.actions["Channels/Green"]);
            p.channelsActionGroup->addAction(p.actions["Channels/Blue"]);
            p.channelsActionGroup->addAction(p.actions["Channels/Alpha"]);
            p.actions["MirrorX"] = new QAction(this);
            p.actions["MirrorX"]->setText(tr("Mirror Horizontal"));
            p.actions["MirrorX"]->setShortcut(QKeySequence(Qt::Key_H));
            p.actions["MirrorX"]->setCheckable(true);
            p.actions["MirrorY"] = new QAction(this);
            p.actions["MirrorY"]->setText(tr("Mirror Vertical"));
            p.actions["MirrorY"]->setShortcut(QKeySequence(Qt::Key_V));
            p.actions["MirrorY"]->setCheckable(true);
            p.actions["VideoLevels/FromFile"] = new QAction(this);
            p.actions["VideoLevels/FromFile"]->setData(QVariant::fromValue<timeline::VideoLevels>(timeline::VideoLevels::FromFile));
            p.actions["VideoLevels/FromFile"]->setCheckable(true);
            p.actions["VideoLevels/FromFile"]->setText(tr("From File"));
            p.actions["VideoLevels/FullRange"] = new QAction(this);
            p.actions["VideoLevels/FullRange"]->setData(QVariant::fromValue<timeline::VideoLevels>(timeline::VideoLevels::FullRange));
            p.actions["VideoLevels/FullRange"]->setCheckable(true);
            p.actions["VideoLevels/FullRange"]->setText(tr("Full Range"));
            p.actions["VideoLevels/LegalRange"] = new QAction(this);
            p.actions["VideoLevels/LegalRange"]->setData(QVariant::fromValue<timeline::VideoLevels>(timeline::VideoLevels::LegalRange));
            p.actions["VideoLevels/LegalRange"]->setCheckable(true);
            p.actions["VideoLevels/LegalRange"]->setText(tr("Legal Range"));
            p.videoLevelsActionGroup = new QActionGroup(this);
            p.videoLevelsActionGroup->addAction(p.actions["VideoLevels/FromFile"]);
            p.videoLevelsActionGroup->addAction(p.actions["VideoLevels/FullRange"]);
            p.videoLevelsActionGroup->addAction(p.actions["VideoLevels/LegalRange"]);
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
            p.alphaBlendActionGroup = new QActionGroup(this);
            p.alphaBlendActionGroup->addAction(p.actions["AlphaBlend/None"]);
            p.alphaBlendActionGroup->addAction(p.actions["AlphaBlend/Straight"]);
            p.alphaBlendActionGroup->addAction(p.actions["AlphaBlend/Premultiplied"]);

            p.menu = new QMenu;
            p.menu->setTitle(tr("&Image"));
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

            _actionsUpdate();

            connect(
                p.actions["MirrorX"],
                &QAction::toggled,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.mirror.x = value;
                    _p->app->setDisplayOptions(displayOptions);
                });
            connect(
                p.actions["MirrorY"],
                &QAction::toggled,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.mirror.y = value;
                    _p->app->setDisplayOptions(displayOptions);
                });

            connect(
                p.videoLevelsActionGroup,
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    auto imageOptions = _p->imageOptions;
                    imageOptions.videoLevels = action->data().value<timeline::VideoLevels>();
                    _p->app->setImageOptions(imageOptions);
                });

            connect(
                p.channelsActionGroup,
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    auto displayOptions = _p->displayOptions;
                    displayOptions.channels = action->data().value<timeline::Channels>() != displayOptions.channels ?
                        action->data().value<timeline::Channels>() :
                        timeline::Channels::Color;
                    _p->app->setDisplayOptions(displayOptions);
                });

            connect(
                _p->alphaBlendActionGroup,
                &QActionGroup::triggered,
                [this](QAction* action)
                {
                    auto imageOptions = _p->imageOptions;
                    imageOptions.alphaBlend = action->data().value<timeline::AlphaBlend>();
                    _p->app->setImageOptions(imageOptions);
                });

            p.filesObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >&)
                {
                    _actionsUpdate();
                });
        }

        ImageActions::~ImageActions()
        {}

        const QMap<QString, QAction*>& ImageActions::actions() const
        {
            return _p->actions;
        }

        QMenu* ImageActions::menu() const
        {
            return _p->menu;
        }

        void ImageActions::setImageOptions(const timeline::ImageOptions& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            _actionsUpdate();
        }

        void ImageActions::setDisplayOptions(const timeline::DisplayOptions& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            _actionsUpdate();
        }

        void ImageActions::_actionsUpdate()
        {
            TLRENDER_P();

            const int count = p.app->filesModel()->observeFiles()->getSize();
            p.actions["VideoLevels/FromFile"]->setEnabled(count > 0);
            p.actions["VideoLevels/FullRange"]->setEnabled(count > 0);
            p.actions["VideoLevels/LegalRange"]->setEnabled(count > 0);
            p.actions["Channels/Red"]->setEnabled(count > 0);
            p.actions["Channels/Green"]->setEnabled(count > 0);
            p.actions["Channels/Blue"]->setEnabled(count > 0);
            p.actions["Channels/Alpha"]->setEnabled(count > 0);
            p.actions["AlphaBlend/None"]->setEnabled(count > 0);
            p.actions["AlphaBlend/Straight"]->setEnabled(count > 0);
            p.actions["AlphaBlend/Premultiplied"]->setEnabled(count > 0);
            p.actions["MirrorX"]->setEnabled(count > 0);
            p.actions["MirrorY"]->setEnabled(count > 0);

            if (count > 0)
            {
                {
                    QSignalBlocker blocker(p.videoLevelsActionGroup);
                    for (auto action : p.videoLevelsActionGroup->actions())
                    {
                        if (action->data().value<timeline::VideoLevels>() == p.imageOptions.videoLevels)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.channelsActionGroup);
                    p.actions["Channels/Red"]->setChecked(false);
                    p.actions["Channels/Green"]->setChecked(false);
                    p.actions["Channels/Blue"]->setChecked(false);
                    p.actions["Channels/Alpha"]->setChecked(false);
                    for (auto action : p.channelsActionGroup->actions())
                    {
                        if (action->data().value<timeline::Channels>() == p.displayOptions.channels)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.alphaBlendActionGroup);
                    for (auto action : p.alphaBlendActionGroup->actions())
                    {
                        if (action->data().value<timeline::AlphaBlend>() == p.imageOptions.alphaBlend)
                        {
                            action->setChecked(true);
                            break;
                        }
                    }
                }
                {
                    QSignalBlocker blocker(p.actions["MirrorX"]);
                    p.actions["MirrorX"]->setChecked(p.displayOptions.mirror.x);
                }
                {
                    QSignalBlocker blocker(p.actions["MirrorY"]);
                    p.actions["MirrorY"]->setChecked(p.displayOptions.mirror.y);
                }
            }
            else
            {
                {
                    QSignalBlocker blocker(p.videoLevelsActionGroup);
                    p.actions["VideoLevels/FromFile"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.channelsActionGroup);
                    p.actions["Channels/Red"]->setChecked(false);
                    p.actions["Channels/Green"]->setChecked(false);
                    p.actions["Channels/Blue"]->setChecked(false);
                    p.actions["Channels/Alpha"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(p.alphaBlendActionGroup);
                    p.actions["AlphaBlend/None"]->setChecked(true);
                }
                {
                    QSignalBlocker blocker(p.actions["MirrorX"]);
                    p.actions["MirrorX"]->setChecked(false);
                }
                {
                    QSignalBlocker blocker(p.actions["MirrorY"]);
                    p.actions["MirrorY"]->setChecked(false);
                }
            }
        }
    }
}
