// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "player.h"

#include <tlTimelineUI/Init.h>
#include <tlTimelineUI/TimelineViewport.h>

#include <tlTimelineGL/Render.h>

#include <dtk/core/CmdLine.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void MainWindow::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<dtk::App>& app)
            {
                dtk::MainWindow::_init(context, app, "player", dtk::Size2I(1280, 720));
            }

            MainWindow::~MainWindow()
            {}

            std::shared_ptr<MainWindow> MainWindow::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<dtk::App>& app)
            {
                auto out = std::shared_ptr<MainWindow>(new MainWindow);
                out->_init(context, app);
                return out;
            }

            std::shared_ptr<dtk::IRender> MainWindow::_createRender(const std::shared_ptr<dtk::Context>& context)
            {
                return timeline_gl::Render::create(context);
            }

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

                auto viewport = timelineui::TimelineViewport::create(_context);
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

int main(int argc, char* argv[])
{
    int r = 1;
    try
    {
        auto context = dtk::Context::create();
        tl::timelineui::init(context);
        auto args = dtk::convert(argc, argv);
        auto app = tl::examples::player::App::create(context, args);
        r = app->getExit();
        if (0 == r)
        {
            app->run();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
