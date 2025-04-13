// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include "MainWindow.h"

#include <dtk/ui/FileBrowser.h>
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
                            "Timeline, movie, or image sequence.",
                            true)
                    });

                _window = MainWindow::create(
                    _context,
                    std::dynamic_pointer_cast<App>(shared_from_this()));
                addWindow(_window);

                if (!_fileName.empty())
                {
                    _open(_fileName);
                }

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

            void App::open()
            {
                auto fileBrowserSystem = _context->getSystem<dtk::FileBrowserSystem>();
                fileBrowserSystem->open(
                    _window,
                    [this](const std::filesystem::path& value)
                    {
                        _open(value.u8string());
                    },
                    dtk::FileBrowserMode::File);
            }

            void App::close()
            {
                _player.reset();
                _window->setPlayer(nullptr);
            }

            void App::reload()
            {
                _open(_fileName);
            }

            void App::_tick()
            {
                if (_player)
                {
                    _player->tick();
                }
            }

            void App::_open(const std::string& fileName)
            {
                _fileName = fileName;
                auto timeline = timeline::Timeline::create(_context, file::Path(fileName));
                _player = timeline::Player::create(_context, timeline);
                _window->setPlayer(_player);
            }
        }
    }
}
