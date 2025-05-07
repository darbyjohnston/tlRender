// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "TabBar.h"

#include "App.h"
#include "FilesModel.h"

namespace tl
{
    namespace play
    {
        void TabBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "TabBar", parent);

            _tabBar = dtk::TabBar::create(context, shared_from_this());
            _tabBar->setTabsClosable(true);

            std::weak_ptr<App> appWeak(app);
            _tabBar->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setCurrent(value);
                    }
                });
            _tabBar->setTabCloseCallback(
                [appWeak](int index)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close(index);
                    }
                });

            _playersObserver = dtk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    const int index = _tabBar->getCurrentTab();
                    _tabBar->clearTabs();
                    for (const auto& player : value)
                    {
                        _tabBar->addTab(
                            player->getPath().get(-1, file::PathType::FileName),
                            player->getPath().get());
                    }
                    _tabBar->setCurrentTab(index);
                });

            _playerIndexObserver = dtk::ValueObserver<int>::create(
                app->getFilesModel()->observePlayerIndex(),
                [this](int value)
                {
                    _tabBar->setCurrentTab(value);
                });
        }

        TabBar::~TabBar()
        {}

        std::shared_ptr<TabBar> TabBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TabBar>(new TabBar);
            out->_init(context, app, parent);
            return out;
        }

        void TabBar::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _tabBar->setGeometry(value);
        }

        void TabBar::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_tabBar->getSizeHint());
        }
    }
}