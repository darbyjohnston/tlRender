// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"
#include "FileActions.h"
#include "MenuBar.h"
#include "PlaybackActions.h"
#include "PlaybackBar.h"
#include "ToolBar.h"
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
                timelineui::Window::_init(context, "player", dtk::Size2I(1280, 720));

                _timeUnitsModel = timeline::TimeUnitsModel::create(context);

                _fileActions = FileActions::create(context, app);
                _playbackActions = PlaybackActions::create(context, app);
                _viewActions = ViewActions::create(context, app);
                _windowActions = WindowActions::create(context, app);

                _layout = dtk::VerticalLayout::create(context, shared_from_this());
                _layout->setSpacingRole(dtk::SizeRole::None);

                _menuBar = MenuBar::create(
                    context,
                    _fileActions->getActions(),
                    _playbackActions->getActions(),
                    _viewActions->getActions(),
                    _windowActions->getActions(),
                    _layout);

                dtk::Divider::create(context, dtk::Orientation::Vertical, _layout);

                _toolBar = ToolBar::create(
                    context,
                    _fileActions->getActions(),
                    _layout);

                _splitter = dtk::Splitter::create(context, dtk::Orientation::Vertical, _layout);

                _viewport = timelineui::Viewport::create(context, _splitter);

                auto vLayout = dtk::VerticalLayout::create(context, _splitter);
                vLayout->setSpacingRole(dtk::SizeRole::None);

                _playbackBar = PlaybackBar::create(
                    context,
                    app,
                    _playbackActions->getActions(),
                    vLayout);

                dtk::Divider::create(context, dtk::Orientation::Vertical, vLayout);

                _timelineWidget = timelineui::TimelineWidget::create(
                    context,
                    _timeUnitsModel,
                    vLayout);
                _timelineWidget->setVStretch(dtk::Stretch::Expanding);
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

            void MainWindow::setPlayer(const std::shared_ptr<timeline::Player>& player)
            {
                _viewport->setPlayer(player);
                _timelineWidget->setPlayer(player);
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
