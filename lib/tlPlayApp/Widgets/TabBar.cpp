// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/TabBar.h>

#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/App.h>

#include <dtk/ui/TabBar.h>

namespace tl
{
    namespace play
    {
        struct TabBar::Private
        {
            int aIndex = -1;
            std::shared_ptr<dtk::TabBar> tabBar;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ValueObserver<int> > aIndexObserver;
        };

        void TabBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::play::TabBar", parent);
            DTK_P();

            p.tabBar = dtk::TabBar::create(context, shared_from_this());
            p.tabBar->setTabsClosable(true);

            std::weak_ptr<App> appWeak(app);
            p.tabBar->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setA(value);
                    }
                });
            p.tabBar->setTabCloseCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close(value);
                    }
                });

            p.filesObserver = dtk::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    DTK_P();
                    p.tabBar->clearTabs();
                    for (const auto& item : value)
                    {
                        p.tabBar->addTab(
                            dtk::elide(item->path.get(-1, file::PathType::FileName)),
                            item->path.get());
                    }
                    p.tabBar->setCurrentTab(p.aIndex);
                });

            p.aIndexObserver = dtk::ValueObserver<int>::create(
                app->getFilesModel()->observeAIndex(),
                [this](int value)
                {
                    DTK_P();
                    p.aIndex = value;
                    p.tabBar->setCurrentTab(value);
                });
        }

        TabBar::TabBar() :
            _p(new Private)
        {}

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
            _p->tabBar->setGeometry(value);
        }

        void TabBar::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->tabBar->getSizeHint());
        }
    }
}
