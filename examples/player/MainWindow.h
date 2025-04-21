// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineWidget.h>
#include <tlTimelineUI/Viewport.h>
#include <tlTimelineUI/Window.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/MainWindow.h>
#include <dtk/ui/MenuBar.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/Splitter.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;
            class CompareActions;
            class FileActions;
            class MenuBar;
            class PlaybackActions;
            class PlaybackBar;
            class SettingsWidget;
            class TabBar;
            class ViewActions;
            class WindowActions;

            //! Main window.
            class MainWindow : public timelineui::Window
            {
                DTK_NON_COPYABLE(MainWindow);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                MainWindow() = default;

            public:
                ~MainWindow();

                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                const std::shared_ptr<timelineui::Viewport>& getViewport() const;

                void showSettings(bool);

                void keyPressEvent(dtk::KeyEvent&) override;
                void keyReleaseEvent(dtk::KeyEvent&) override;

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
                std::shared_ptr<SettingsWidget> _settingsWidget;
                std::shared_ptr<dtk::Splitter> _splitter;
                std::shared_ptr<dtk::Splitter> _splitter2;
                std::shared_ptr<dtk::VerticalLayout> _layout;
                std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
                std::shared_ptr<dtk::ValueObserver<timeline::Compare> > _compareObserver;
            };
        }
    }
}
