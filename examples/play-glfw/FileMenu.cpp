// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "FileMenu.h"

#include "App.h"

#include <tlUI/EventLoop.h>
#include <tlUI/FileBrowser.h>

#include <tlCore/File.h>

#if defined(TLRENDER_NFD)
#include <nfd.hpp>
#endif // TLRENDER_NFD

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct FileMenu::Private
            {
                std::weak_ptr<App> app;
                std::shared_ptr<timeline::Player> player;

                std::shared_ptr<Menu> recentMenu;
                std::shared_ptr<Menu> currentMenu;
                std::shared_ptr<ui::FileBrowser> fileBrowser;

                std::shared_ptr<observer::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            };

            void FileMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                p.app = app;

                auto appWeak = std::weak_ptr<App>(app);
                auto item = std::make_shared<ui::MenuItem>(
                    "Open",
                    "FileOpen",
                    ui::Key::O,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this, appWeak]
                    {
                        close();
                        _openFile();
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Open With Separate Audio",
                    ui::Key::O,
                    static_cast<int>(ui::KeyModifier::Shift) |
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Close",
                    ui::Key::E,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this, appWeak]
                    {
                        close();
                        if (auto app = appWeak.lock())
                        {
                            app->closeAll();
                        }
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Close All",
                    ui::Key::E,
                    static_cast<int>(ui::KeyModifier::Shift) |
                    static_cast<int>(ui::KeyModifier::Control),
                    [this, appWeak]
                    {
                        close();
                        if (auto app = appWeak.lock())
                        {
                            app->closeAll();
                        }
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Reload",
                    [this]
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                p.recentMenu = addSubMenu("Recent");
                for (size_t i = 0; i < 10; ++i)
                {
                    item = std::make_shared<ui::MenuItem>(
                        "File Name",
                        [this]
                        {
                            close();
                        });
                    p.recentMenu->addItem(item);
                    p.recentMenu->setItemEnabled(item, false);
                }

                addDivider();

                p.currentMenu = addSubMenu("Current");
                for (size_t i = 0; i < 10; ++i)
                {
                    item = std::make_shared<ui::MenuItem>(
                        "File Name",
                        [this]
                        {
                            close();
                        });
                    p.currentMenu->addItem(item);
                    p.currentMenu->setItemEnabled(item, false);
                }

                item = std::make_shared<ui::MenuItem>(
                    "Next",
                    "Next",
                    ui::Key::PageDown,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Previous",
                    "Prev",
                    ui::Key::PageUp,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                addDivider();

                item = std::make_shared<ui::MenuItem>(
                    "Next Layer",
                    ui::Key::Equal,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                item = std::make_shared<ui::MenuItem>(
                    "Previous Layer",
                    "Prev",
                    ui::Key::Minus,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        close();
                    });
                addItem(item);
                setItemEnabled(item, false);

                addDivider();

                item = std::make_shared<ui::MenuItem>(
                    "Exit",
                    ui::Key::Q,
                    static_cast<int>(ui::KeyModifier::Control),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->exit();
                        }
                    });
                addItem(item);

                p.playerObserver = observer::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    {
                        _p->player = value;
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

            void FileMenu::_openFile()
            {
                TLRENDER_P();
/*#if defined(TLRENDER_NFD)
                if (auto app = p.app.lock())
                {
                    nfdu8char_t* outPath = nullptr;
                    NFD::OpenDialog(outPath);
                    if (outPath)
                    {
                        app->open(outPath);
                        NFD::FreePath(outPath);
                    }
                }
#else  // TLRENDER_NFD*/
                if (auto app = p.app.lock())
                {
                    if (auto context = _context.lock())
                    {
                        std::string path;
                        if (p.player)
                        {
                            path = p.player->getPath().get();
                        }
                        else
                        {
                            path = file::getCWD();
                        }
                        p.fileBrowser = ui::FileBrowser::create(path, context);
                        p.fileBrowser->open(app->getEventLoop());
                        p.fileBrowser->setFileCallback(
                            [this](const std::string& value)
                            {
                                if (auto app = _p->app.lock())
                                {
                                    app->open(value);
                                }
                                _p->fileBrowser->close();
                            });
                        p.fileBrowser->setCloseCallback(
                            [this]
                            {
                                _p->fileBrowser.reset();
                            });
                    }
                }
//#endif // TLRENDER_NFD
            }
        }
    }
}
