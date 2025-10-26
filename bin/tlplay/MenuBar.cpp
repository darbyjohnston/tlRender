// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MenuBar.h"

#include "App.h"
#include "CompareActions.h"
#include "FileActions.h"
#include "FilesModel.h"
#include "PlaybackActions.h"
#include "RecentFilesModel.h"
#include "ViewActions.h"
#include "WindowActions.h"

#include "App.h"

namespace tl
{
    namespace play
    {
        void FileMenu::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::Menu::_init(context, parent);

            auto actions = fileActions->getActions();
            addAction(actions["Open"]);
            addAction(actions["Close"]);
            addAction(actions["CloseAll"]);
            addAction(actions["Reload"]);
            _recentFilesMenu = addSubMenu("Recent Files");
            addDivider();
            _filesMenu = addSubMenu("Files");
            addAction(actions["Next"]);
            addAction(actions["Prev"]);
            addDivider();
            addAction(actions["Exit"]);

            std::weak_ptr<App> appWeak(app);
            _playersObserver = ftk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayers(),
                [this, appWeak](const std::vector<std::shared_ptr<timeline::Player> >& players)
                {
                    _filesActions.clear();
                    _filesMenu->clear();
                    for (size_t i = 0; i < players.size(); ++i)
                    {
                        auto action = ftk::Action::create(
                            players[i]->getPath().get(-1, file::PathType::FileName),
                            [this, appWeak, i]
                            {
                                if (auto app = appWeak.lock())
                                {
                                    app->getFilesModel()->setCurrent(i);
                                }
                                close();
                            });
                        action->setChecked(i == _playerIndex);
                        _filesActions.push_back(action);
                        _filesMenu->addAction(action);
                    }
                });

            _playerIndexObserver = ftk::ValueObserver<int>::create(
                app->getFilesModel()->observePlayerIndex(),
                [this](int value)
                {
                    _playerIndex = value;
                    for (size_t i = 0; i < _filesActions.size(); ++i)
                    {
                        _filesActions[i]->setChecked(i == value);
                    }
                });

            _recentFilesObserver = ftk::ListObserver<std::filesystem::path>::create(
                app->getRecentFilesModel()->observeRecent(),
                [this, appWeak](const std::vector<std::filesystem::path>& value)
                {
                    _recentFilesActions.clear();
                    _recentFilesMenu->clear();
                    for (auto i = value.rbegin(); i != value.rend(); ++i)
                    {
                        const std::filesystem::path path = *i;
                        auto action = ftk::Action::create(
                            path.filename().u8string(),
                            [this, appWeak, path]
                            {
                                if (auto app = appWeak.lock())
                                {
                                    app->open(path);
                                }
                            });
                        _recentFilesActions.push_back(action);
                        _recentFilesMenu->addAction(action);
                    }
                });
        }

        FileMenu::~FileMenu()
        {}

        std::shared_ptr<FileMenu> FileMenu::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileMenu>(new FileMenu);
            out->_init(context, app, fileActions, parent);
            return out;
        }

        void CompareMenu::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::Menu::_init(context, parent);

            _bFileMenu = addSubMenu("B File");
            auto actions = compareActions->getActions();
            for (const auto& label : timeline::getCompareLabels())
            {
                addAction(actions[label]);
            }

            std::weak_ptr<App> appWeak(app);
            _playersObserver = ftk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayers(),
                [this, appWeak](const std::vector<std::shared_ptr<timeline::Player> >& players)
                {
                    _bFileActions.clear();
                    _bFileMenu->clear();
                    for (size_t i = 0; i < players.size(); ++i)
                    {
                        auto action = ftk::Action::create(
                            players[i]->getPath().get(-1, file::PathType::FileName),
                            [this, appWeak, i](bool value)
                            {
                                close();
                                if (auto app = appWeak.lock())
                                {
                                    app->getFilesModel()->setB(value ? i : -1);
                                }
                            });
                        action->setChecked(i == _bPlayerIndex);
                        _bFileActions.push_back(action);
                        _bFileMenu->addAction(action);
                    }
                });

            _bPlayerIndexObserver = ftk::ValueObserver<int>::create(
                app->getFilesModel()->observeBPlayerIndex(),
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
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
            out->_init(context, app, compareActions, parent);
            return out;
        }

        void PlaybackMenu::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<PlaybackActions>& playbackActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::Menu::_init(context, parent);
            auto actions = playbackActions->getActions();
            addAction(actions["Stop"]);
            addAction(actions["Forward"]);
            addAction(actions["Reverse"]);
            addAction(actions["TogglePlayback"]);
            addDivider();
            addAction(actions["Start"]);
            addAction(actions["Prev"]);
            addAction(actions["Next"]);
            addAction(actions["End"]);
            addDivider();
            addAction(actions["SetInPoint"]);
            addAction(actions["ResetInPoint"]);
            addAction(actions["SetOutPoint"]);
            addAction(actions["ResetOutPoint"]);
        }

        PlaybackMenu::~PlaybackMenu()
        {}

        std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<PlaybackActions>& playbackActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
            out->_init(context, playbackActions, parent);
            return out;
        }

        void ViewMenu::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::Menu::_init(context, parent);
            auto actions = viewActions->getActions();
            addAction(actions["Frame"]);
            addAction(actions["ZoomReset"]);
            addAction(actions["ZoomIn"]);
            addAction(actions["ZoomOut"]);
        }

        ViewMenu::~ViewMenu()
        {}

        std::shared_ptr<ViewMenu> ViewMenu::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
            out->_init(context, viewActions, parent);
            return out;
        }

        void WindowMenu::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::Menu::_init(context, parent);
            auto actions = windowActions->getActions();
            addAction(actions["FullScreen"]);
            addDivider();
            addAction(actions["1920x1080"]);
            addAction(actions["3840x2160"]);
            addDivider();
            addAction(actions["Settings"]);
        }

        WindowMenu::~WindowMenu()
        {}

        std::shared_ptr<WindowMenu> WindowMenu::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
            out->_init(context, windowActions, parent);
            return out;
        }

        void MenuBar::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<PlaybackActions>& playbackActions,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ftk::MenuBar::_init(context, parent);
            addMenu("File", FileMenu::create(context, app, fileActions));
            addMenu("Compare", CompareMenu::create(context, app, compareActions));
            addMenu("Playback", PlaybackMenu::create(context, playbackActions));
            addMenu("View", ViewMenu::create(context, viewActions));
            addMenu("Window", WindowMenu::create(context, windowActions));
        }

        MenuBar::~MenuBar()
        {}

        std::shared_ptr<MenuBar> MenuBar::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<PlaybackActions>& playbackActions,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MenuBar>(new MenuBar);
            out->_init(
                context,
                app,
                fileActions,
                compareActions,
                playbackActions,
                viewActions,
                windowActions,
                parent);
            return out;
        }
    }
}