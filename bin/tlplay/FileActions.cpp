// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "FileActions.h"

#include "App.h"
#include "FilesModel.h"

namespace tl
{
    namespace play
    {
        void FileActions::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto appWeak = std::weak_ptr<App>(app);
            _actions["Open"] = feather_tk::Action::create(
                "Open",
                "FileOpen",
                feather_tk::Key::O,
                static_cast<int>(feather_tk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->open();
                    }
                });
            _actions["Open"]->setTooltip("Open a file.");

            _actions["Close"] = feather_tk::Action::create(
                "Close",
                "FileClose",
                feather_tk::Key::E,
                static_cast<int>(feather_tk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });
            _actions["Close"]->setTooltip("Close the current file.");

            _actions["CloseAll"] = feather_tk::Action::create(
                "Close All",
                "FileCloseAll",
                feather_tk::Key::E,
                static_cast<int>(feather_tk::commandKeyModifier) | static_cast<int>(feather_tk::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });
            _actions["CloseAll"]->setTooltip("Close all files.");

            _actions["Reload"] = feather_tk::Action::create(
                "Reload",
                "FileReload",
                feather_tk::Key::R,
                static_cast<int>(feather_tk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->reload();
                    }
                });
            _actions["Reload"]->setTooltip("Reload the current file.");

            _actions["Next"] = feather_tk::Action::create(
                "Next",
                "Next",
                feather_tk::Key::PageDown,
                static_cast<int>(feather_tk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->next();
                    }
                });

            _actions["Prev"] = feather_tk::Action::create(
                "Previous",
                "Prev",
                feather_tk::Key::PageUp,
                static_cast<int>(feather_tk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prev();
                    }
                });

            _actions["Exit"] = feather_tk::Action::create(
                "Exit",
                feather_tk::Key::Q,
                static_cast<int>(feather_tk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->exit();
                    }
                });

            _playersObserver = feather_tk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _actions["Next"]->setEnabled(value.size() > 1);
                    _actions["Prev"]->setEnabled(value.size() > 1);
                });

            _playerObserver = feather_tk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _actions["Close"]->setEnabled(value.get());
                    _actions["CloseAll"]->setEnabled(value.get());
                    _actions["Reload"]->setEnabled(value.get());
                });
        }

        FileActions::~FileActions()
        {
        }

        std::shared_ptr<FileActions> FileActions::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<FileActions>(new FileActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& FileActions::getActions() const
        {
            return _actions;
        }
    }
}