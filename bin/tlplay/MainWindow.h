// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineWidget.h>
#include <tlTimelineUI/Viewport.h>
#include <tlTimelineUI/Window.h>

#include <tlTimeline/Player.h>

#include <feather-tk/ui/MainWindow.h>
#include <feather-tk/ui/MenuBar.h>
#include <feather-tk/ui/RowLayout.h>
#include <feather-tk/ui/Splitter.h>

namespace tl
{
    namespace play
    {
        class App;
        class CompareActions;
        class FileActions;
        class MenuBar;
        class PlaybackActions;
        class PlaybackBar;
        class SettingsWidget;
        class StatusBar;
        class TabBar;
        class ViewActions;
        class WindowActions;

        //! Main window.
        class MainWindow : public timelineui::Window
        {
            FTK_NON_COPYABLE(MainWindow);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&);

            MainWindow() = default;

        public:
            ~MainWindow();

            static std::shared_ptr<MainWindow> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&);

            const std::shared_ptr<timelineui::Viewport>& getViewport() const;

            void showSettings(bool);

            void keyPressEvent(ftk::KeyEvent&) override;
            void keyReleaseEvent(ftk::KeyEvent&) override;

        protected:
            void _drop(const std::vector<std::string>&) override;

        private:
            std::weak_ptr<App> _app;
            std::shared_ptr<timelineui::Viewport> _viewport;
            std::shared_ptr<FileActions> _fileActions;
            std::shared_ptr<CompareActions> _compareActions;
            std::shared_ptr<PlaybackActions> _playbackActions;
            std::shared_ptr<ViewActions> _viewActions;
            std::shared_ptr<WindowActions> _windowActions;
            std::shared_ptr<MenuBar> _menuBar;
            std::shared_ptr<TabBar> _tabBar;
            std::shared_ptr<PlaybackBar> _playbackBar;
            std::shared_ptr<timelineui::TimelineWidget> _timelineWidget;
            std::shared_ptr<StatusBar> _statusBar;
            std::shared_ptr<SettingsWidget> _settingsWidget;
            std::shared_ptr<ftk::Splitter> _splitter;
            std::shared_ptr<ftk::Splitter> _splitter2;
            std::shared_ptr<ftk::VerticalLayout> _layout;
            std::shared_ptr<ftk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
            std::shared_ptr<ftk::ValueObserver<timeline::Compare> > _compareObserver;
        };
    }
}