// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"
#include "FileActions.h"
#include "MenuBar.h"
#include "PlaybackActions.h"
#include "PlaybackBar.h"
#include "TabBar.h"
#include "ToolBars.h"
#include "ViewActions.h"
#include "WindowActions.h"

#include <dtk/ui/Divider.h>
#include <dtk/ui/Menu.h>
#include <dtk/ui/ToolBar.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void MainWindow::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                timelineui::Window::_init(context, "player", dtk::Size2I(1920, 1080));

                _viewport = timelineui::Viewport::create(context);

                _fileActions = FileActions::create(context, app);
                _windowActions = WindowActions::create(
                    context,
                    app,
                    std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
                _viewActions = ViewActions::create(
                    context,
                    app,
                    std::dynamic_pointer_cast<MainWindow>(shared_from_this()));
                _playbackActions = PlaybackActions::create(context, app);

                _menuBar = MenuBar::create(
                    context,
                    app,
                    _fileActions,
                    _windowActions,
                    _viewActions,
                    _playbackActions);

                auto toolBars = ToolBars::create(
                    context,
                    _fileActions,
                    _windowActions,
                    _viewActions);

                _tabBar = TabBar::create(context, app);

                _playbackBar = PlaybackBar::create(
                    context,
                    app,
                    _playbackActions->getActions());

                _timelineWidget = timelineui::TimelineWidget::create(
                    context,
                    app->getTimeUnitsModel());
                _timelineWidget->setVStretch(dtk::Stretch::Expanding);

                _layout = dtk::VerticalLayout::create(context, shared_from_this());
                _layout->setSpacingRole(dtk::SizeRole::None);
                _menuBar->setParent(_layout);
                dtk::Divider::create(context, dtk::Orientation::Vertical, _layout);
                toolBars->setParent(_layout);
                dtk::Divider::create(context, dtk::Orientation::Vertical, _layout);
                _splitter = dtk::Splitter::create(context, dtk::Orientation::Vertical, _layout);
                auto vLayout = dtk::VerticalLayout::create(context, _splitter);
                vLayout->setSpacingRole(dtk::SizeRole::None);
                _tabBar->setParent(vLayout);
                _viewport->setParent(vLayout);
                vLayout = dtk::VerticalLayout::create(context, _splitter);
                vLayout->setSpacingRole(dtk::SizeRole::None);
                _playbackBar->setParent(vLayout);
                dtk::Divider::create(context, dtk::Orientation::Vertical, vLayout);
                _timelineWidget->setParent(vLayout);

                _playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    {
                        _viewport->setPlayer(value);
                        _timelineWidget->setPlayer(value);
                    });
            }

            MainWindow::~MainWindow()
            {}

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

            void MainWindow::keyPressEvent(dtk::KeyEvent& event)
            {
                event.accept = _menuBar->shortcut(event.key, event.modifiers);
            }

            void MainWindow::keyReleaseEvent(dtk::KeyEvent& event)
            {
                event.accept = true;
            }
        }
    }
}
