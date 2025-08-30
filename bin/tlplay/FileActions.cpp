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
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto appWeak = std::weak_ptr<App>(app);
            _actions["Open"] = ftk::Action::create(
                "Open",
                "FileOpen",
                ftk::Key::O,
                static_cast<int>(ftk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->open();
                    }
                });
            _actions["Open"]->setTooltip("Open a file.");

            _actions["Close"] = ftk::Action::create(
                "Close",
                "FileClose",
                ftk::Key::E,
                static_cast<int>(ftk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });
            _actions["Close"]->setTooltip("Close the current file.");

            _actions["CloseAll"] = ftk::Action::create(
                "Close All",
                "FileCloseAll",
                ftk::Key::E,
                static_cast<int>(ftk::commandKeyModifier) | static_cast<int>(ftk::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });
            _actions["CloseAll"]->setTooltip("Close all files.");

            _actions["Reload"] = ftk::Action::create(
                "Reload",
                "FileReload",
                ftk::Key::R,
                static_cast<int>(ftk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->reload();
                    }
                });
            _actions["Reload"]->setTooltip("Reload the current file.");

            _actions["Next"] = ftk::Action::create(
                "Next",
                "Next",
                ftk::Key::PageDown,
                static_cast<int>(ftk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->next();
                    }
                });

            _actions["Prev"] = ftk::Action::create(
                "Previous",
                "Prev",
                ftk::Key::PageUp,
                static_cast<int>(ftk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prev();
                    }
                });

            _actions["Exit"] = ftk::Action::create(
                "Exit",
                ftk::Key::Q,
                static_cast<int>(ftk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->exit();
                    }
                });

            _playersObserver = ftk::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _actions["Next"]->setEnabled(value.size() > 1);
                    _actions["Prev"]->setEnabled(value.size() > 1);
                });

            _playerObserver = ftk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
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
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<FileActions>(new FileActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ftk::Action> >& FileActions::getActions() const
        {
            return _actions;
        }
    }
}