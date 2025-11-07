// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "App.h"

#include "FilesModel.h"
#include "MainWindow.h"
#include "RecentFilesModel.h"
#include "SettingsModel.h"

#include <tlTimeline/Util.h>

#include <ftk/UI/DialogSystem.h>
#include <ftk/UI/FileBrowser.h>
#include <ftk/Core/File.h>

namespace tl
{
    namespace play
    {
        void App::_init(
            const std::shared_ptr<ftk::Context>& context,
            std::vector<std::string>& argv)
        {
            _cmdLine.inputs = ftk::CmdLineListArg<std::string>::create(
                "input",
                "One or more timelines, movies, or image sequences.",
                true);

            ftk::App::_init(
                context,
                argv,
                "tlplay",
                "Example player application.",
                { _cmdLine.inputs });
        }

        App::~App()
        {}

        std::shared_ptr<App> App::create(
            const std::shared_ptr<ftk::Context>& context,
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
                auto dialogSystem = _context->getSystem<ftk::DialogSystem>();
                dialogSystem->message("ERROR", e.what(), _window);
            }
            _recentFilesModel->addRecent(path);
        }

        void App::open()
        {
            auto fileBrowserSystem = _context->getSystem<ftk::FileBrowserSystem>();
            fileBrowserSystem->open(
                _window,
                [this](const std::filesystem::path& value)
                {
                    open(value);
                });
        }

        void App::reload()
        {
            try
            {
                _filesModel->reload();
            }
            catch (const std::exception& e)
            {
                auto dialogSystem = _context->getSystem<ftk::DialogSystem>();
                dialogSystem->message("ERROR", e.what(), _window);
            }
        }

        void App::run()
        {
            _context->getSystem<ftk::FileBrowserSystem>()->setNativeFileDialog(false);

            _settingsModel = SettingsModel::create(
                _context,
                ftk::getSettingsPath("tlRender", "tlplay.json"));

            _timeUnitsModel = timeline::TimeUnitsModel::create(_context);

            _recentFilesModel = RecentFilesModel::create(_context, _settingsModel->getSettings());
            auto fileBrowserSystem = _context->getSystem<ftk::FileBrowserSystem>();
            fileBrowserSystem->getModel()->setExtensions(timeline::getExtensions(_context));
            fileBrowserSystem->setRecentFilesModel(_recentFilesModel);

            _filesModel = FilesModel::create(_context, _settingsModel);

#if defined(TLRENDER_BMD)
            _bmdOutputDevice = bmd::OutputDevice::create(_context);
            bmd::DeviceConfig bmdConfig;
            bmdConfig.deviceIndex = 0;
            bmdConfig.displayModeIndex = 3;
            bmdConfig.pixelType = bmd::PixelType::_8BitBGRA;
            _bmdOutputDevice->setConfig(bmdConfig);
            _bmdOutputDevice->setEnabled(true);
#endif // TLRENDER_BMD

            _window = MainWindow::create(
                _context,
                std::dynamic_pointer_cast<App>(shared_from_this()));

            _playerObserver = ftk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                _filesModel->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
#if defined(TLRENDER_BMD)
                    _bmdOutputDevice->setPlayer(value);
#endif // TLRENDER_BMD
                });

            for (const auto& input : _cmdLine.inputs->getList())
            {
                open(std::filesystem::u8path(input));
            }

            ftk::App::run();
        }

        void App::_tick()
        {
            _filesModel->tick();
#if defined(TLRENDER_BMD)
            if (_bmdOutputDevice)
            {
                _bmdOutputDevice->tick();
            }
#endif // TLRENDER_BMD
        }
    }
}