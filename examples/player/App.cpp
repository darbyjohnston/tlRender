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

                _timeUnitsModel = timeline::TimeUnitsModel::create(context);

                _player = dtk::ObservableValue<std::shared_ptr<timeline::Player> >::create();

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

            const std::shared_ptr<timeline::TimeUnitsModel>& App::getTimeUnitsModel() const
            {
                return _timeUnitsModel;
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
                _player->setIfChanged(nullptr);
            }

            void App::reload()
            {
                _open(_fileName);
            }

            std::shared_ptr<dtk::IObservableValue<std::shared_ptr<timeline::Player> > > App::observePlayer() const
            {
                return _player;
            }

            void App::_tick()
            {
                if (auto player = _player->get())
                {
                    player->tick();
                }
            }

            void App::_open(const std::string& fileName)
            {
                _fileName = fileName;
                auto timeline = timeline::Timeline::create(_context, file::Path(fileName));
                auto player = timeline::Player::create(_context, timeline);
                _player->setIfChanged(player);
            }
        }
    }
}
