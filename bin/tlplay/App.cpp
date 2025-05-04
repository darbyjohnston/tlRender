// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include "FilesModel.h"
#include "MainWindow.h"
#include "RecentFilesModel.h"
#include "SettingsModel.h"

#include <tlTimeline/Util.h>

#include <dtk/ui/DialogSystem.h>
#include <dtk/ui/FileBrowser.h>
#include <dtk/core/CmdLine.h>
#include <dtk/core/File.h>

namespace tl
{
    namespace play
    {
        void App::_init(
            const std::shared_ptr<dtk::Context>& context,
            std::vector<std::string>& argv)
        {
            dtk::App::_init(
                context,
                argv,
                "tlplay",
                "Example player application.",
                {
                    dtk::CmdLineListArg<std::string>::create(
                        _cmdLineOptions.fileNames,
                        "input",
                        "Timelines, movies, or image sequences.",
                        true)
                });

            context->getSystem<dtk::FileBrowserSystem>()->setNativeFileDialog(false);

            _settingsModel = SettingsModel::create(
                context,
                dtk::getSettingsPath("tlRender", "tlplay.json"));

            _timeUnitsModel = timeline::TimeUnitsModel::create(context);

            _recentFilesModel = RecentFilesModel::create(context, _settingsModel->getSettings());
            auto fileBrowserSystem = _context->getSystem<dtk::FileBrowserSystem>();
            fileBrowserSystem->setExtensions(timeline::getExtensions(_context));
            fileBrowserSystem->setRecentFilesModel(_recentFilesModel);

            _filesModel = FilesModel::create(context, _settingsModel);

            _window = MainWindow::create(
                _context,
                std::dynamic_pointer_cast<App>(shared_from_this()));
            addWindow(_window);

            for (const auto& fileName : _cmdLineOptions.fileNames)
            {
                open(std::filesystem::u8path(fileName));
            }

            _window->show();
        }

        App::~App()
        {
        }

        std::shared_ptr<App> App::create(
            const std::shared_ptr<dtk::Context>& context,
            std::vector<std::string>& argv)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(context, argv);
            return out;
        }

        const std::shared_ptr<SettingsModel>& App::getSettingsModel() const
        {
            return _settingsModel;
        }

        const std::shared_ptr<timeline::TimeUnitsModel>& App::getTimeUnitsModel() const
        {
            return _timeUnitsModel;
        }

        const std::shared_ptr<RecentFilesModel>& App::getRecentFilesModel() const
        {
            return _recentFilesModel;
        }

        const std::shared_ptr<FilesModel>& App::getFilesModel() const
        {
            return _filesModel;
        }

        void App::open(const std::filesystem::path& path)
        {
            try
            {
                _filesModel->open(path);
            }
            catch (const std::exception& e)
            {
                auto dialogSystem = _context->getSystem<dtk::DialogSystem>();
                dialogSystem->message("ERROR", e.what(), _window);
            }
            _recentFilesModel->addRecent(path);
        }

        void App::open()
        {
            auto fileBrowserSystem = _context->getSystem<dtk::FileBrowserSystem>();
            fileBrowserSystem->open(
                _window,
                [this](const std::filesystem::path& value)
                {
                    open(value);
                },
                std::filesystem::path(),
                dtk::FileBrowserMode::File);
        }

        void App::reload()
        {
            try
            {
                _filesModel->reload();
            }
            catch (const std::exception& e)
            {
                auto dialogSystem = _context->getSystem<dtk::DialogSystem>();
                dialogSystem->message("ERROR", e.what(), _window);
            }
        }

        void App::_tick()
        {
            _filesModel->tick();
        }
    }
}