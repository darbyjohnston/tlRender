// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/TimeUnits.h>

#include <dtk/ui/App.h>

namespace tl
{
    namespace play
    {
        class FilesModel;
        class MainWindow;
        class RecentFilesModel;
        class SettingsModel;

        //! Application.
        class App : public dtk::App
        {
            DTK_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                std::vector<std::string>&);

            App() = default;

        public:
            ~App();

            static std::shared_ptr<App> create(
                const std::shared_ptr<dtk::Context>&,
                std::vector<std::string>&);

            const std::shared_ptr<SettingsModel>& getSettingsModel() const;
            const std::shared_ptr<timeline::TimeUnitsModel>& getTimeUnitsModel() const;
            const std::shared_ptr<RecentFilesModel>& getRecentFilesModel() const;
            const std::shared_ptr<FilesModel>& getFilesModel() const;

            void open(const std::filesystem::path&);
            void open();
            void reload();

        protected:
            void _tick() override;

        private:
            struct CmdLineOptions
            {
                std::vector<std::string> fileNames;
            };
            CmdLineOptions _cmdLineOptions;
            std::shared_ptr<SettingsModel> _settingsModel;
            std::shared_ptr<timeline::TimeUnitsModel> _timeUnitsModel;
            std::shared_ptr<RecentFilesModel> _recentFilesModel;
            std::shared_ptr<FilesModel> _filesModel;
            std::shared_ptr<MainWindow> _window;
        };
    }
}