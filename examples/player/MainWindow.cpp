// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "App.h"
#include "MenuBar.h"
#include "PlaybackBar.h"
#include "ToolBar.h"

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

                _menuBar = MenuBar::create(context, app);

                _toolBar = ToolBar::create(context, _menuBar->getActions());

                _viewport = timelineui::Viewport::create(context);

                _playbackBar = PlaybackBar::create(context, app);

                _timelineWidget = timelineui::TimelineWidget::create(context, _timeUnitsModel);
                _timelineWidget->setVStretch(dtk::Stretch::Expanding);

                _layout = dtk::VerticalLayout::create(context, shared_from_this());
                _layout->setSpacingRole(dtk::SizeRole::None);
                _menuBar->setParent(_layout);
                dtk::Divider::create(context, dtk::Orientation::Vertical, _layout);
                _toolBar->setParent(_layout);
                _splitter = dtk::Splitter::create(context, dtk::Orientation::Vertical, _layout);
                _viewport->setParent(_splitter);
                auto vLayout = dtk::VerticalLayout::create(context, _splitter);
                vLayout->setSpacingRole(dtk::SizeRole::None);
                _playbackBar->setParent(vLayout);
                _timelineWidget->setParent(vLayout);
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
