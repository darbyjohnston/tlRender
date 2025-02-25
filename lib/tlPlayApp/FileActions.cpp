// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FileActions.h>

#include <tlPlayApp/App.h>

#include <tlPlay/FilesModel.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play_app
    {
        struct FileActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void FileActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Open"] = std::make_shared<dtk::Action>(
                "Open",
                "FileOpen",
                dtk::Key::O,
                static_cast<int>(dtk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openDialog();
                    }
                });
            p.actions["Open"]->toolTip = dtk::Format(
                "Open a file\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Open"]->shortcut,
                    p.actions["Open"]->shortcutModifiers));

            p.actions["OpenSeparateAudio"] = std::make_shared<dtk::Action>(
                "Open With Separate Audio",
                "FileOpenSeparateAudio",
                dtk::Key::O,
                static_cast<int>(dtk::KeyModifier::Shift) |
                static_cast<int>(dtk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openSeparateAudioDialog();
                    }
                });
            p.actions["OpenSeparateAudio"]->toolTip = dtk::Format(
                "Open a file with separate audio\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["OpenSeparateAudio"]->shortcut,
                    p.actions["OpenSeparateAudio"]->shortcutModifiers));

            p.actions["Close"] = std::make_shared<dtk::Action>(
                "Close",
                "FileClose",
                dtk::Key::E,
                static_cast<int>(dtk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });
            p.actions["Close"]->toolTip = dtk::Format(
                "Close the current file\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Close"]->shortcut,
                    p.actions["Close"]->shortcutModifiers));

            p.actions["CloseAll"] = std::make_shared<dtk::Action>(
                "Close All",
                "FileCloseAll",
                dtk::Key::E,
                static_cast<int>(dtk::KeyModifier::Shift) |
                static_cast<int>(dtk::commandKeyModifier),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });
            p.actions["CloseAll"]->toolTip = dtk::Format(
                "Close all files\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Close"]->shortcut,
                    p.actions["Close"]->shortcutModifiers));

            p.actions["Reload"] = std::make_shared<dtk::Action>(
                "Reload",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->reload();
                    }
                });

            p.actions["Next"] = std::make_shared<dtk::Action>(
                "Next",
                "Next",
                dtk::Key::PageDown,
                static_cast<int>(dtk::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->next();
                    }
                });

            p.actions["Prev"] = std::make_shared<dtk::Action>(
                "Previous",
                "Prev",
                dtk::Key::PageUp,
                static_cast<int>(dtk::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prev();
                    }
                });

            p.actions["NextLayer"] = std::make_shared<dtk::Action>(
                "Next Layer",
                "Next",
                dtk::Key::Equal,
                static_cast<int>(dtk::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextLayer();
                    }
                });

            p.actions["PrevLayer"] = std::make_shared<dtk::Action>(
                "Previous Layer",
                "Prev",
                dtk::Key::Minus,
                static_cast<int>(dtk::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevLayer();
                    }
                });

            p.actions["Exit"] = std::make_shared<dtk::Action>(
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
    }
}
