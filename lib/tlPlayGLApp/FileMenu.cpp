// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/FileMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct FileMenu::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<ui::MenuItem> > items;
            std::shared_ptr<Menu> recentMenu;
            std::vector<std::shared_ptr<ui::MenuItem> > recentItems;
            std::shared_ptr<Menu> currentMenu;
            std::vector<std::shared_ptr<ui::MenuItem> > currentItems;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<play::FilesModelItem> > > aObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<file::Path> > recentObserver;
        };

        void FileMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            p.app = app;

            auto appWeak = std::weak_ptr<App>(app);
            p.items["Open"] = std::make_shared<ui::MenuItem>(
                "Open",
                "FileOpen",
                ui::Key::O,
                static_cast<int>(ui::commandKeyModifier),
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->openDialog();
                    }
                });
            addItem(p.items["Open"]);

            p.items["OpenSeparateAudio"] = std::make_shared<ui::MenuItem>(
                "Open With Separate Audio",
                "FileOpenSeparateAudio",
                ui::Key::O,
                static_cast<int>(ui::KeyModifier::Shift) |
                static_cast<int>(ui::commandKeyModifier),
                [this]
                {
                    close();
                });
            addItem(p.items["OpenSeparateAudio"]);
            setItemEnabled(p.items["OpenSeparateAudio"], false);

            p.items["Close"] = std::make_shared<ui::MenuItem>(
                "Close",
                "FileClose",
                ui::Key::E,
                static_cast<int>(ui::commandKeyModifier),
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });
            addItem(p.items["Close"]);

            p.items["CloseAll"] = std::make_shared<ui::MenuItem>(
                "Close All",
                "FileCloseAll",
                ui::Key::E,
                static_cast<int>(ui::KeyModifier::Shift) |
                static_cast<int>(ui::commandKeyModifier),
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });
            addItem(p.items["CloseAll"]);

            p.items["Reload"] = std::make_shared<ui::MenuItem>(
                "Reload",
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->reload();
                    }
                });
            addItem(p.items["Reload"]);

            p.recentMenu = addSubMenu("Recent");

            addDivider();

            p.currentMenu = addSubMenu("Current");

            p.items["Next"] = std::make_shared<ui::MenuItem>(
                "Next",
                "Next",
                ui::Key::PageDown,
                static_cast<int>(ui::KeyModifier::Control),
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->next();
                    }
                });
            addItem(p.items["Next"]);

            p.items["Prev"] = std::make_shared<ui::MenuItem>(
                "Previous",
                "Prev",
                ui::Key::PageUp,
                static_cast<int>(ui::KeyModifier::Control),
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prev();
                    }
                });
            addItem(p.items["Prev"]);

            addDivider();

            p.items["NextLayer"] = std::make_shared<ui::MenuItem>(
                "Next Layer",
                "Next",
                ui::Key::Equal,
                static_cast<int>(ui::KeyModifier::Control),
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextLayer();
                    }
                });
            addItem(p.items["NextLayer"]);

            p.items["PrevLayer"] = std::make_shared<ui::MenuItem>(
                "Previous Layer",
                "Prev",
                ui::Key::Minus,
                static_cast<int>(ui::KeyModifier::Control),
                [this, appWeak]
                {
                    close();
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevLayer();
                    }
                });
            addItem(p.items["PrevLayer"]);

            addDivider();

            p.items["Exit"] = std::make_shared<ui::MenuItem>(
                "Exit",
                ui::Key::Q,
                static_cast<int>(ui::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->exit();
                    }
                });
            addItem(p.items["Exit"]);

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
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<FileMenu>(new FileMenu);
            out->_init(app, context);
            return out;
        }

        void FileMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.recentMenu->close();
            p.currentMenu->close();
        }

        void FileMenu::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();

            setItemEnabled(p.items["Close"], !value.empty());
            setItemEnabled(p.items["CloseAll"], !value.empty());
            setItemEnabled(p.items["Reload"], !value.empty());
            setItemEnabled(p.items["Next"], value.size() > 1);
            setItemEnabled(p.items["Prev"], value.size() > 1);

            p.currentMenu->clear();
            p.currentItems.clear();
            for (size_t i = 0; i < value.size(); ++i)
            {
                auto item = std::make_shared<ui::MenuItem>(
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
            setItemEnabled(p.items["NextLayer"], value ? value->videoLayers.size() > 1 : false);
            setItemEnabled(p.items["PrevLayer"], value ? value->videoLayers.size() > 1 : false);
        }

        void FileMenu::_aIndexUpdate(int value)
        {
            TLRENDER_P();
            for (int i = 0; i < p.currentItems.size(); ++i)
            {
                p.currentMenu->setItemChecked(p.currentItems[i], i == value);
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
                auto item = std::make_shared<ui::MenuItem>(
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
