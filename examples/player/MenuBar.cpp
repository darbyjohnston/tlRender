// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MenuBar.h"

#include "App.h"
#include "CompareActions.h"
#include "FileActions.h"
#include "PlaybackActions.h"
#include "ViewActions.h"
#include "WindowActions.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void FileMenu::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::Menu::_init(context, parent);

                auto actions = fileActions->getActions();
                addItem(actions["Open"]);
                addItem(actions["Close"]);
                addItem(actions["CloseAll"]);
                addItem(actions["Reload"]);
                addDivider();
                _filesMenu = addSubMenu("Files");
                addItem(actions["Next"]);
                addItem(actions["Prev"]);
                addDivider();
                addItem(actions["Exit"]);

                std::weak_ptr<App> appWeak(app);
                _playersObserver = dtk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayers(),
                    [this, appWeak](const std::vector<std::shared_ptr<timeline::Player> >& players)
                    {
                        _filesActions.clear();
                        _filesMenu->clear();
                        for (size_t i = 0; i < players.size(); ++i)
                        {
                            auto action = dtk::Action::create(
                                players[i]->getPath().get(-1, file::PathType::FileName),
                                [this, appWeak, i]
                                {
                                    close();
                                    if (auto app = appWeak.lock())
                                    {
                                        app->setCurrent(i);
                                    }
                                });
                            action->setChecked(i == _playerIndex);
                            _filesActions.push_back(action);
                            _filesMenu->addItem(action);
                        }
                    });

                _playerIndexObserver = dtk::ValueObserver<int>::create(
                    app->observePlayerIndex(),
                    [this](int value)
                    {
                        _playerIndex = value;
                        for (size_t i = 0; i < _filesActions.size(); ++i)
                        {
                            _filesActions[i]->setChecked(i == value);
                        }
                    });
            }

            FileMenu::~FileMenu()
            {}

            std::shared_ptr<FileMenu> FileMenu::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<FileMenu>(new FileMenu);
                out->_init(context, app, fileActions, parent);
                return out;
            }

            void CompareMenu::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<CompareActions>& compareActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::Menu::_init(context, parent);

                _bFileMenu = addSubMenu("B File");
                auto actions = compareActions->getActions();
                for (const auto& label : timeline::getCompareLabels())
                {
                    addItem(actions[label]);
                }

                std::weak_ptr<App> appWeak(app);
                _playersObserver = dtk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayers(),
                    [this, appWeak](const std::vector<std::shared_ptr<timeline::Player> >& players)
                    {
                        _bFileActions.clear();
                        _bFileMenu->clear();
                        for (size_t i = 0; i < players.size(); ++i)
                        {
                            auto action = dtk::Action::create(
                                players[i]->getPath().get(-1, file::PathType::FileName),
                                [this, appWeak, i](bool value)
                                {
                                    close();
                                    if (auto app = appWeak.lock())
                                    {
                                        app->setB(value ? i : -1);
                                    }
                                });
                            action->setChecked(i == _bPlayerIndex);
                            _bFileActions.push_back(action);
                            _bFileMenu->addItem(action);
                        }
                    });

                _bPlayerIndexObserver = dtk::ValueObserver<int>::create(
                    app->observeBPlayerIndex(),
                    [this](int value)
                    {
                        _bPlayerIndex = value;
                        for (size_t i = 0; i < _bFileActions.size(); ++i)
                        {
                            _bFileActions[i]->setChecked(i == value);
                        }
                    });
            }

            CompareMenu::~CompareMenu()
            {}

            std::shared_ptr<CompareMenu> CompareMenu::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<CompareActions>& compareActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
                out->_init(context, app, compareActions, parent);
                return out;
            }

            void WindowMenu::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::Menu::_init(context, parent);
                auto actions = windowActions->getActions();
                addItem(actions["FullScreen"]);
                addDivider();
                addItem(actions["1920x1080"]);
                addItem(actions["3840x2160"]);
            }

            WindowMenu::~WindowMenu()
            {}

            std::shared_ptr<WindowMenu> WindowMenu::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
                out->_init(context, windowActions, parent);
                return out;
            }

            void ViewMenu::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<ViewActions>& viewActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::Menu::_init(context, parent);
                auto actions = viewActions->getActions();
                addItem(actions["Frame"]);
            }

            ViewMenu::~ViewMenu()
            {}

            std::shared_ptr<ViewMenu> ViewMenu::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<ViewActions>& viewActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
                out->_init(context, viewActions, parent);
                return out;
            }

            void PlaybackMenu::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<PlaybackActions>& playbackActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::Menu::_init(context, parent);
                auto actions = playbackActions->getActions();
                addItem(actions["Stop"]);
                addItem(actions["Forward"]);
                addItem(actions["Reverse"]);
                addItem(actions["TogglePlayback"]);
                addDivider();
                addItem(actions["Start"]);
                addItem(actions["Prev"]);
                addItem(actions["Next"]);
                addItem(actions["End"]);
            }

            PlaybackMenu::~PlaybackMenu()
            {}

            std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<PlaybackActions>& playbackActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
                out->_init(context, playbackActions, parent);
                return out;
            }

            void MenuBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<CompareActions>& compareActions,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<ViewActions>& viewActions,
                const std::shared_ptr<PlaybackActions>& playbackActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::MenuBar::_init(context, parent);
                addMenu("File", FileMenu::create(context, app, fileActions));
                addMenu("Compare", CompareMenu::create(context, app, compareActions));
                addMenu("Window", WindowMenu::create(context, windowActions));
                addMenu("View", ViewMenu::create(context, viewActions));
                addMenu("Playback", PlaybackMenu::create(context, playbackActions));
            }

            MenuBar::~MenuBar()
            {}

            std::shared_ptr<MenuBar> MenuBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<CompareActions>& compareActions,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<ViewActions>& viewActions,
                const std::shared_ptr<PlaybackActions>& playbackActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MenuBar>(new MenuBar);
                out->_init(
                    context,
                    app,
                    fileActions,
                    compareActions,
                    windowActions,
                    viewActions,
                    playbackActions,
                    parent);
                return out;
            }
        }
    }
}
