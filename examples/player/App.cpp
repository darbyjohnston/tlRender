// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlTimelineUI/Viewport.h>

#include <dtk/core/CmdLine.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void App::_init(
                const std::shared_ptr<dtk::Context>& context,
                std::vector<std::string>& argv)
            {
                dtk::App::_init(
                    context,
                    argv,
                    "player",
                    "Example player application.",
                    {
                        dtk::CmdLineValueArg<std::string>::create(
                            _fileName,
                            "input",
                            "Timeline, movie, or image sequence.")
                    });

                auto timeline = timeline::Timeline::create(_context, file::Path(_fileName));
                _player = timeline::Player::create(_context, timeline);
                _player->setPlayback(timeline::Playback::Forward);

                _window = MainWindow::create(
                    _context,
                    std::dynamic_pointer_cast<App>(shared_from_this()));
                addWindow(_window);

                auto viewport = timelineui::Viewport::create(_context);
                timeline::BackgroundOptions backgroundOptions;
                backgroundOptions.type = timeline::Background::Checkers;
                viewport->setBackgroundOptions(backgroundOptions);
                viewport->setPlayer(_player);
                _window->setWidget(viewport);

                _window->show();
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                const std::shared_ptr<dtk::Context>& context,
                std::vector<std::string>& argv)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(context, argv);
                return out;
            }

            void App::_tick()
            {
                _player->tick();
            }
        }
    }
}
