// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/FileMenu.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/RecentFilesModel.h>

namespace tl
{
    namespace play_gl
    {
        struct FileMenu::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::shared_ptr<Menu> recentMenu;
            std::vector<std::shared_ptr<ui::Action> > recentItems;
            std::shared_ptr<Menu> currentMenu;
            std::vector<std::shared_ptr<ui::Action> > currentItems;
            std::shared_ptr<Menu> layersMenu;
            std::vector<std::shared_ptr<ui::Action> > layersItems;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<play::FilesModelItem> > > aObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ListObserver<file::Path> > recentObserver;
        };

        void FileMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.app = app;

            p.actions = actions;
            addItem(p.actions["Open"]);
            addItem(p.actions["OpenSeparateAudio"]);
            addItem(p.actions["Close"]);
            addItem(p.actions["CloseAll"]);
            addItem(p.actions["Reload"]);
            p.recentMenu = addSubMenu("Recent");
            addDivider();
            p.currentMenu = addSubMenu("Current");
            addItem(p.actions["Next"]);
            addItem(p.actions["Prev"]);
            addDivider();
            p.layersMenu = addSubMenu("Layers");
            addItem(p.actions["NextLayer"]);
            addItem(p.actions["PrevLayer"]);
            addDivider();
            addItem(p.actions["Exit"]);

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });

            p.aObserver = observer::ValueObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeA(),
                [this](const std::shared_ptr<play::FilesModelItem>& value)
                {
                    _aUpdate(value);
                });

            p.aIndexObserver = observer::ValueObserver<int>::create(
                app->getFilesModel()->observeAIndex(),
                [this](int value)
                {
                    _aIndexUpdate(value);
                });

            p.layersObserver = observer::ListObserver<int>::create(
                app->getFilesModel()->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    _layersUpdate(value);
                });

            p.recentObserver = observer::ListObserver<file::Path>::create(
                app->getRecentFilesModel()->observeRecent(),
                [this](const std::vector<file::Path>& value)
                {
                    _recentUpdate(value);
                });
        }

        FileMenu::FileMenu() :
            _p(new Private)
        {}

        FileMenu::~FileMenu()
        {}

        std::shared_ptr<FileMenu> FileMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileMenu>(new FileMenu);
            out->_init(actions, app, context, parent);
            return out;
        }

        void FileMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.recentMenu->close();
            p.currentMenu->close();
            p.layersMenu->close();
        }

        void FileMenu::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();

            setItemEnabled(p.actions["Close"], !value.empty());
            setItemEnabled(p.actions["CloseAll"], !value.empty());
            setItemEnabled(p.actions["Reload"], !value.empty());
            setItemEnabled(p.actions["Next"], value.size() > 1);
            setItemEnabled(p.actions["Prev"], value.size() > 1);

            p.currentMenu->clear();
            p.currentItems.clear();
            for (size_t i = 0; i < value.size(); ++i)
            {
                auto item = std::make_shared<ui::Action>(
                    value[i]->path.get(-1, false),
                    [this, i]
                    {
                        close();
                        if (auto app = _p->app.lock())
                        {
                            app->getFilesModel()->setA(i);
                        }
                    });
                p.currentMenu->addItem(item);
                p.currentItems.push_back(item);
            }
        }

        void FileMenu::_aUpdate(const std::shared_ptr<play::FilesModelItem>& value)
        {
            TLRENDER_P();

            p.layersMenu->clear();
            p.layersItems.clear();
            if (value)
            {
                for (size_t i = 0; i < value->videoLayers.size(); ++i)
                {
                    auto item = std::make_shared<ui::Action>(
                        value->videoLayers[i],
                        [this, value, i]
                        {
                            close();
                        if (auto app = _p->app.lock())
                        {
                            app->getFilesModel()->setLayer(value, i);
                        }
                        });
                    item->checked = i == value->videoLayer;
                    p.layersMenu->addItem(item);
                    p.layersItems.push_back(item);
                }
            }

            setItemEnabled(p.actions["NextLayer"], value ? value->videoLayers.size() > 1 : false);
            setItemEnabled(p.actions["PrevLayer"], value ? value->videoLayers.size() > 1 : false);
        }

        void FileMenu::_aIndexUpdate(int value)
        {
            TLRENDER_P();
            for (int i = 0; i < p.currentItems.size(); ++i)
            {
                p.currentMenu->setItemChecked(p.currentItems[i], i == value);
            }
        }

        void FileMenu::_layersUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (auto app = p.app.lock())
            {
                auto a = app->getFilesModel()->getA();
                for (size_t i = 0; i < p.layersItems.size(); ++i)
                {
                    p.layersMenu->setItemChecked(p.layersItems[i], i == a->videoLayer);
                }
            }
        }

        void FileMenu::_recentUpdate(const std::vector<file::Path>& value)
        {
            TLRENDER_P();

            p.recentMenu->clear();
            p.recentItems.clear();
            for (size_t i = 0; i < value.size(); ++i)
            {
                const auto path = value[i];
                auto item = std::make_shared<ui::Action>(
                    path.get(-1, false),
                    [this, path]
                    {
                        close();
                        if (auto app = _p->app.lock())
                        {
                            app->open(path.get());
                        }
                    });
                p.recentMenu->addItem(item);
                p.recentItems.push_back(item);
            }
        }
    }
}
