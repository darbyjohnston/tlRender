// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/FileActions.h>

#include <tlPlayApp/Models/FilesModel.h>
#include <tlPlayApp/App.h>

namespace tl
{
    namespace play
    {
        struct FileActions::Private
        {
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<FilesModelItem> > > aObserver;
        };

        void FileActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            IActions::_init(context, app, "File");
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            _actions["Open"] = dtk::Action::create(
                "Open",
                "FileOpen",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openDialog();
                    }
                });

            _actions["OpenSeparateAudio"] = dtk::Action::create(
                "Open With Separate Audio",
                "FileOpenSeparateAudio",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openSeparateAudioDialog();
                    }
                });

            _actions["Close"] = dtk::Action::create(
                "Close",
                "FileClose",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });

            _actions["CloseAll"] = dtk::Action::create(
                "Close All",
                "FileCloseAll",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });

            _actions["Reload"] = dtk::Action::create(
                "Reload",
                "FileReload",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->reload();
                    }
                });

            _actions["Next"] = dtk::Action::create(
                "Next",
                "Next",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->next();
                    }
                });

            _actions["Prev"] = dtk::Action::create(
                "Previous",
                "Prev",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prev();
                    }
                });

            _actions["NextLayer"] = dtk::Action::create(
                "Next Layer",
                "Next",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->nextLayer();
                    }
                });

            _actions["PrevLayer"] = dtk::Action::create(
                "Previous Layer",
                "Prev",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->prevLayer();
                    }
                });

            _actions["Exit"] = dtk::Action::create(
                "Exit",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->exit();
                    }
                });

            _tooltips =
            {
                { "Open", "Open a file." },
                { "OpenSeparateAudio", "Open a file with separate audio." },
                { "Close", "Close the current file." },
                { "CloseAll", "Close all files." },
                { "Reload", "Reload the current file." },
                { "Next", "Change to the next file." },
                { "Prev", "Change to the previous file." },
                { "NextLayer", "Change to the next layer." },
                { "PrevLayer", "Change to the previous layer." },
                { "Exit", "Exit the application." }
            };

            _keyShortcutsUpdate(app->getSettingsModel()->getKeyShortcuts());

            p.filesObserver = dtk::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    DTK_P();
                    _actions["Close"]->setEnabled(!value.empty());
                    _actions["CloseAll"]->setEnabled(!value.empty());
                    _actions["Reload"]->setEnabled(!value.empty());
                    _actions["Next"]->setEnabled(value.size() > 1);
                    _actions["Prev"]->setEnabled(value.size() > 1);
                });

            p.aObserver = dtk::ValueObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeA(),
                [this](const std::shared_ptr<FilesModelItem>& value)
                {
                    _actions["NextLayer"]->setEnabled(value ? value->videoLayers.size() > 1 : false);
                    _actions["PrevLayer"]->setEnabled(value ? value->videoLayers.size() > 1 : false);
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
    }
}
