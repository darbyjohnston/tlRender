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

                context->getSystem<dtk::FileBrowserSystem>()->setNativeFileDialog(false);

                _timeUnitsModel = timeline::TimeUnitsModel::create(context);

                _players = dtk::ObservableList<std::shared_ptr<timeline::Player> >::create();
                _player = dtk::ObservableValue<std::shared_ptr<timeline::Player> >::create();
                _playerIndex = dtk::ObservableValue<int>::create(-1);

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
                close(_players->indexOf(_player->get()));
            }

            void App::close(int index)
            {
                if (index >= 0 && index < _players->getSize())
                {
                    if (auto player = _player->get())
                    {
                        std::size_t j = _players->indexOf(player);
                        _players->removeItem(index);
                        if (j > 0 && index <= j)
                        {
                            --j;
                        }
                        _player->setIfChanged(!_players->isEmpty() ? _players->getItem(j) : nullptr);
                        _playerIndex->setIfChanged(!_players->isEmpty() ? j : -1);
                    }
                }
            }

            void App::closeAll()
            {
                _players->clear();
                _player->setIfChanged(nullptr);
                _playerIndex->setIfChanged(-1);
            }

            void App::reload()
            {
                if (auto player = _player->get())
                {
                    const std::size_t index = _players->indexOf(player);
                    const file::Path path = player->getPath();
                    auto timeline = timeline::Timeline::create(_context, path);
                    player = timeline::Player::create(_context, timeline);
                    _players->setItem(index, player);
                    _player->setIfChanged(player);
                }
            }

            void App::setCurrent(int value)
            {
                if (value >= 0 && value < _players->getSize())
                {
                    _player->setIfChanged(_players->getItem(value));
                    _playerIndex->setIfChanged(value);
                }
            }

            void App::next()
            {
                if (auto player = _player->get())
                {
                    std::size_t index = _players->indexOf(player) + 1;
                    if (index >= _players->getSize())
                    {
                        index = 0;
                    }
                    setCurrent(index);
                }
            }

            void App::prev()
            {
                if (auto player = _player->get())
                {
                    std::size_t index = _players->indexOf(player);
                    if (index > 0)
                    {
                        --index;
                    }
                    else
                    {
                        index = _players->getSize() - 1;
                    }
                    setCurrent(index);
                }
            }

            std::shared_ptr<dtk::IObservableList<std::shared_ptr<timeline::Player> > > App::observePlayers() const
            {
                return _players;
            }

            std::shared_ptr<dtk::IObservableValue<std::shared_ptr<timeline::Player> > > App::observePlayer() const
            {
                return _player;
            }

            std::shared_ptr<dtk::IObservableValue<int> > App::observePlayerIndex() const
            {
                return _playerIndex;
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
                const int index = _players->getSize();
                _players->pushBack(player);
                _player->setIfChanged(player);
                _playerIndex->setIfChanged(index);
            }
        }
    }
}
