// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>
#include <tlTimeline/TimeUnits.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <ftk/UI/App.h>
#include <ftk/Core/CmdLine.h>

namespace tl
{
    namespace play
    {
        class FilesModel;
        class MainWindow;
        class RecentFilesModel;
        class SettingsModel;

        //! Application.
        class App : public ftk::App
        {
            FTK_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                std::vector<std::string>&);

            App() = default;

        public:
            ~App();

            static std::shared_ptr<App> create(
                const std::shared_ptr<ftk::Context>&,
                std::vector<std::string>&);

            const std::shared_ptr<SettingsModel>& getSettingsModel() const;
            const std::shared_ptr<timeline::TimeUnitsModel>& getTimeUnitsModel() const;
            const std::shared_ptr<RecentFilesModel>& getRecentFilesModel() const;
            const std::shared_ptr<FilesModel>& getFilesModel() const;

            void open(const std::filesystem::path&);
            void open();
            void reload();

            void run() override;

        protected:
            void _tick() override;

        private:
            struct CmdLine
            {
                std::shared_ptr<ftk::CmdLineListArg<std::string> > inputs;
            };
            CmdLine _cmdLine;

            std::shared_ptr<SettingsModel> _settingsModel;
            std::shared_ptr<timeline::TimeUnitsModel> _timeUnitsModel;
            std::shared_ptr<RecentFilesModel> _recentFilesModel;
            std::shared_ptr<FilesModel> _filesModel;

            std::shared_ptr<MainWindow> _window;

#if defined(TLRENDER_BMD)
            std::shared_ptr<bmd::OutputDevice> _bmdOutputDevice;
#endif // TLRENDER_BMD

            std::shared_ptr<ftk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
        };
    }
}