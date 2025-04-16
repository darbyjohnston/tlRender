// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "FileActions.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void FileActions::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto appWeak = std::weak_ptr<App>(app);
                _actions["Open"] = dtk::Action::create(
                    "Open",
                    "FileOpen",
                    dtk::Key::O,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->open();
                        }
                    });
                _actions["Open"]->setTooltip("Open a file.");

                _actions["Close"] = dtk::Action::create(
                    "Close",
                    "FileClose",
                    dtk::Key::E,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->close();
                        }
                    });
                _actions["Close"]->setTooltip("Close the current file.");

                _actions["Reload"] = dtk::Action::create(
                    "Reload",
                    "FileReload",
                    dtk::Key::R,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->reload();
                        }
                    });
                _actions["Reload"]->setTooltip("Reload the current file.");

                _actions["Exit"] = dtk::Action::create(
                    "Exit",
                    dtk::Key::Q,
                    static_cast<int>(dtk::commandKeyModifier),
                    [appWeak]
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->exit();
                        }
                    });

                _playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    {
                        _actions["Close"]->setEnabled(value.get());
                        _actions["Reload"]->setEnabled(value.get());
                    });
            }

            FileActions::~FileActions()
            {}

            std::shared_ptr<FileActions> FileActions::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto out = std::shared_ptr<FileActions>(new FileActions);
                out->_init(context, app);
                return out;
            }

            const std::map<std::string, std::shared_ptr<dtk::Action> >& FileActions::getActions() const
            {
                return _actions;
            }
        }
    }
}
