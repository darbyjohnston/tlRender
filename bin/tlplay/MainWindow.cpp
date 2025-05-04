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
#include "TabBar.h"
#include "ToolBars.h"
#include "ViewActions.h"
#include "WindowActions.h"

#include <dtk/ui/Divider.h>
#include <dtk/ui/Menu.h>
#include <dtk/ui/ToolBar.h>

namespace tl
{
    namespace play
    {
        void MainWindow::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            timelineui::Window::_init(context, "tlplay", dtk::Size2I(1920, 1080));

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
            _timelineWidget->setVStretch(dtk::Stretch::Expanding);

            _settingsWidget = SettingsWidget::create(context, app);
            _settingsWidget->hide();

            _layout = dtk::VerticalLayout::create(context, shared_from_this());
            _layout->setSpacingRole(dtk::SizeRole::None);
            _menuBar->setParent(_layout);
            dtk::Divider::create(context, dtk::Orientation::Vertical, _layout);
            toolBars->setParent(_layout);
            dtk::Divider::create(context, dtk::Orientation::Vertical, _layout);
            _splitter = dtk::Splitter::create(context, dtk::Orientation::Vertical, _layout);
            _splitter2 = dtk::Splitter::create(context, dtk::Orientation::Horizontal, _splitter);
            auto vLayout = dtk::VerticalLayout::create(context, _splitter2);
            vLayout->setSpacingRole(dtk::SizeRole::None);
            _tabBar->setParent(vLayout);
            _viewport->setParent(vLayout);
            _settingsWidget->setParent(_splitter2);
            vLayout = dtk::VerticalLayout::create(context, _splitter);
            vLayout->setSpacingRole(dtk::SizeRole::None);
            _playbackBar->setParent(vLayout);
            dtk::Divider::create(context, dtk::Orientation::Vertical, vLayout);
            _timelineWidget->setParent(vLayout);

            _playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _viewport->setPlayer(value);
                    _timelineWidget->setPlayer(value);
                });

            _compareObserver = dtk::ValueObserver<timeline::Compare>::create(
                app->getFilesModel()->observeCompare(),
                [this](timeline::Compare value)
                {
                    timeline::CompareOptions options;
                    options.compare = value;
                    _viewport->setCompareOptions(options);
                });
        }

        MainWindow::~MainWindow()
        {
        }

        std::shared_ptr<MainWindow> MainWindow::create(
            const std::shared_ptr<dtk::Context>& context,
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

        void MainWindow::keyPressEvent(dtk::KeyEvent& event)
        {
            event.accept = _menuBar->shortcut(event.key, event.modifiers);
        }

        void MainWindow::keyReleaseEvent(dtk::KeyEvent& event)
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