// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "player.h"

#include <tlTimelineUI/Init.h>
#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void App::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::vector<std::string>& argv)
            {
                ui_app::App::_init(
                    context,
                    argv,
                    "player",
                    "Example player application.",
                    {
                        app::CmdLineValueArg<std::string>::create(
                            _fileName,
                            "input",
                            "Timeline, movie, or image sequence.")
                    });
                const int exitCode = getExit();
                if (exitCode != 0)
                {
                    exit(exitCode);
                    return;
                }

                auto timeline = timeline::Timeline::create(context, file::Path(_fileName));
                _player = timeline::Player::create(context, timeline);
                _player->setPlayback(timeline::Playback::Forward);

                _window = ui_app::Window::create(context, "player");
                addWindow(_window);

                auto viewport = timelineui::TimelineViewport::create(context, _window);
                timeline::BackgroundOptions backgroundOptions;
                backgroundOptions.type = timeline::Background::Checkers;
                viewport->setBackgroundOptions(backgroundOptions);
                viewport->setPlayer(_player);

                _window->show();
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::vector<std::string>& argv)
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

int main(int argc, char* argv[])
{
    int r = 1;
    try
    {
        auto context = dtk::Context::create();
        tl::timelineui::init(context);
        auto app = tl::examples::player::App::create(context, tl::app::convert(argc, argv));
        r = app->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
