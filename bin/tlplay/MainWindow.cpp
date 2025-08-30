// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"
#include "CompareActions.h"
#include "FileActions.h"
#include "FilesModel.h"
#include "MenuBar.h"
#include "PlaybackActions.h"
#include "PlaybackBar.h"
#include "SettingsWidget.h"
#include "StatusBar.h"
#include "TabBar.h"
#include "ToolBars.h"
#include "ViewActions.h"
#include "WindowActions.h"

#include <feather-tk/ui/Divider.h>
#include <feather-tk/ui/Menu.h>
#include <feather-tk/ui/ToolBar.h>

namespace tl
{
    namespace play
    {
        void MainWindow::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            timelineui::Window::_init(context, "tlplay", ftk::Size2I(1920, 1080));

            _app = app;

            _viewport = timelineui::Viewport::create(context);

            _fileActions = FileActions::create(context, app);
            _compareActions = CompareActions::create(context, app);
            _playbackActions = PlaybackActions::create(context, app);
            _viewActions = ViewActions::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
            _windowActions = WindowActions::create(
                context,
                app,
                std::dynamic_pointer_cast<MainWindow>(shared_from_this()));

            _menuBar = MenuBar::create(
                context,
                app,
                _fileActions,
                _compareActions,
                _playbackActions,
                _viewActions,
                _windowActions);

            auto toolBars = ToolBars::create(
                context,
                _fileActions,
                _compareActions,
                _viewActions,
                _windowActions);

            _tabBar = TabBar::create(context, app);

            _playbackBar = PlaybackBar::create(
                context,
                app,
                _playbackActions->getActions());

            _timelineWidget = timelineui::TimelineWidget::create(
                context,
                app->getTimeUnitsModel());
            _timelineWidget->setVStretch(ftk::Stretch::Expanding);

            _statusBar = StatusBar::create(context, app);

            _settingsWidget = SettingsWidget::create(context, app);
            _settingsWidget->hide();

            _layout = ftk::VerticalLayout::create(context, shared_from_this());
            _layout->setSpacingRole(ftk::SizeRole::None);
            _menuBar->setParent(_layout);
            ftk::Divider::create(context, ftk::Orientation::Vertical, _layout);
            toolBars->setParent(_layout);
            ftk::Divider::create(context, ftk::Orientation::Vertical, _layout);
            _splitter = ftk::Splitter::create(context, ftk::Orientation::Vertical, _layout);
            _splitter2 = ftk::Splitter::create(context, ftk::Orientation::Horizontal, _splitter);
            auto vLayout = ftk::VerticalLayout::create(context, _splitter2);
            vLayout->setSpacingRole(ftk::SizeRole::None);
            _tabBar->setParent(vLayout);
            _viewport->setParent(vLayout);
            _settingsWidget->setParent(_splitter2);
            vLayout = ftk::VerticalLayout::create(context, _splitter);
            vLayout->setSpacingRole(ftk::SizeRole::None);
            _playbackBar->setParent(vLayout);
            ftk::Divider::create(context, ftk::Orientation::Vertical, vLayout);
            _timelineWidget->setParent(vLayout);
            ftk::Divider::create(context, ftk::Orientation::Vertical, vLayout);
            _statusBar->setParent(vLayout);

            _playerObserver = ftk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _viewport->setPlayer(value);
                    _timelineWidget->setPlayer(value);
                });

            _compareObserver = ftk::ValueObserver<timeline::Compare>::create(
                app->getFilesModel()->observeCompare(),
                [this](timeline::Compare value)
                {
                    timeline::CompareOptions options;
                    options.compare = value;
                    _viewport->setCompareOptions(options);
                });
        }

        MainWindow::~MainWindow()
        {}

        std::shared_ptr<MainWindow> MainWindow::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<MainWindow>(new MainWindow);
            out->_init(context, app);
            return out;
        }

        const std::shared_ptr<timelineui::Viewport>& MainWindow::getViewport() const
        {
            return _viewport;
        }

        void MainWindow::showSettings(bool value)
        {
            _settingsWidget->setVisible(value);
        }

        void MainWindow::keyPressEvent(ftk::KeyEvent& event)
        {
            event.accept = _menuBar->shortcut(event.key, event.modifiers);
        }

        void MainWindow::keyReleaseEvent(ftk::KeyEvent& event)
        {
            event.accept = true;
        }

        void MainWindow::_drop(const std::vector<std::string>& value)
        {
            if (auto app = _app.lock())
            {
                for (const auto& fileName : value)
                {
                    app->open(fileName);
                }
            }
        }
    }
}
