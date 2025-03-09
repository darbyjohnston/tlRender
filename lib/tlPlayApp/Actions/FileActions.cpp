// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/FileActions.h>

#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/App.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct FileActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void FileActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Open"] = dtk::Action::create(
                "Open",
                "FileOpen",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openDialog();
                    }
                });

            p.actions["OpenSeparateAudio"] = dtk::Action::create(
                "Open With Separate Audio",
                "FileOpenSeparateAudio",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openSeparateAudioDialog();
                    }
                });

            p.actions["Close"] = dtk::Action::create(
                "Close",
                "FileClose",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });

            p.actions["CloseAll"] = dtk::Action::create(
                "Close All",
                "FileCloseAll",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });

            p.actions["Reload"] = dtk::Action::create(
                "Reload",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->reload();
                    }
                });

            p.actions["Next"] = dtk::Action::create(
                "Next",
                "Next",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->next();
                    }
                });

            p.actions["Prev"] = dtk::Action::create(
                "Previous",
                "Prev",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prev();
                    }
                });

            p.actions["NextLayer"] = dtk::Action::create(
                "Next Layer",
                "Next",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextLayer();
                    }
                });

            p.actions["PrevLayer"] = dtk::Action::create(
                "Previous Layer",
                "Prev",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevLayer();
                    }
                });

            p.actions["Exit"] = dtk::Action::create(
                "Exit",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->exit();
                    }
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
                });
        }

        FileActions::FileActions() :
            _p(new Private)
        {}

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
            return _p->actions;
        }

        void FileActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            const std::map<std::string, std::string> tooltips =
            {
                {
                    "Open",
                    "Open a file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "OpenSeparateAudio",
                    "Open a file with separate audio.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Close",
                    "Close the current file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "CloseAll",
                    "Close all files.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Reload",
                    "Reload the current file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Next",
                    "Change to the next file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Prev",
                    "Change to the previous file.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "NextLayer",
                    "Change to the next layer.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "PrevLayer",
                    "Change to the previous layer.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Exit",
                    "Exit the application.\n"
                    "\n"
                    "Shortcut: {0}"
                }
            };
            for (const auto& i : p.actions)
            {
                auto j = value.shortcuts.find(dtk::Format("File/{0}").arg(i.first));
                if (j != value.shortcuts.end())
                {
                    i.second->setShortcut(j->second.key);
                    i.second->setShortcutModifiers(j->second.modifiers);
                    const auto k = tooltips.find(i.first);
                    if (k != tooltips.end())
                    {
                        i.second->setTooltip(dtk::Format(k->second).
                            arg(dtk::getShortcutLabel(j->second.key, j->second.modifiers)));
                    }
                }
            }
        }
    }
}
