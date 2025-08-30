// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include "FilesModel.h"

#include "SettingsModel.h"

namespace tl
{
    namespace play
    {
        void FilesModel::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<SettingsModel>& settingsModel)
        {
            _context = context;
            _players = ftk::ObservableList<std::shared_ptr<timeline::Player> >::create();
            _player = ftk::ObservableValue<std::shared_ptr<timeline::Player> >::create();
            _playerIndex = ftk::ObservableValue<int>::create(-1);
            _bPlayer = ftk::ObservableValue<std::shared_ptr<timeline::Player> >::create();
            _bPlayerIndex = ftk::ObservableValue<int>::create(-1);
            _compare = ftk::ObservableValue<timeline::Compare>::create(timeline::Compare::A);

            _cacheObserver = ftk::ValueObserver<timeline::PlayerCacheOptions>::create(
                settingsModel->observeCache(),
                [this](const timeline::PlayerCacheOptions& value)
                {
                    for (const auto& player : _players->get())
                    {
                        player->setCacheOptions(value);
                    }
                });
        }

        FilesModel::~FilesModel()
        {
        }

        std::shared_ptr<FilesModel> FilesModel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<SettingsModel>& settingsModel)
        {
            auto out = std::shared_ptr<FilesModel>(new FilesModel);
            out->_init(context, settingsModel);
            return out;
        }

        void FilesModel::open(const std::filesystem::path& path)
        {
            if (auto context = _context.lock())
            {
                auto timeline = timeline::Timeline::create(context, file::Path(path.u8string()));
                auto player = timeline::Player::create(context, timeline);
                const int index = _players->getSize();
                _players->pushBack(player);
                _player->setIfChanged(player);
                _playerIndex->setIfChanged(index);
            }
        }

        void FilesModel::close()
        {
            close(_players->indexOf(_player->get()));
        }

        void FilesModel::close(int index)
        {
            if (index >= 0 && index < _players->getSize())
            {
                if (auto player = _player->get())
                {
                    std::size_t j = _players->indexOf(player);
                    if (j > 0 && index <= j)
                    {
                        --j;
                    }
                    std::size_t k = _players->indexOf(_bPlayer->get());
                    if (k != ftk::ObservableListInvalidIndex && k > 0 && index <= k)
                    {
                        --k;
                    }

                    _players->removeItem(index);
                    _player->setIfChanged(
                        !_players->isEmpty() && j != ftk::ObservableListInvalidIndex ? _players->getItem(j) : nullptr);
                    _playerIndex->setIfChanged(
                        !_players->isEmpty() && j != ftk::ObservableListInvalidIndex ? j : -1);
                    _bPlayer->setIfChanged(
                        !_players->isEmpty() && k != ftk::ObservableListInvalidIndex ? _players->getItem(k) : nullptr);
                    _bPlayerIndex->setIfChanged(
                        !_players->isEmpty() && k != ftk::ObservableListInvalidIndex ? k : -1);

                    if ((player = _player->get()))
                    {
                        if (auto bPlayer = _bPlayer->get())
                        {
                            player->setCompare({ bPlayer->getTimeline() });
                        }
                        else
                        {
                            player->setCompare({});
                        }
                    }
                }
            }
        }

        void FilesModel::closeAll()
        {
            _players->clear();
            _player->setIfChanged(nullptr);
            _playerIndex->setIfChanged(-1);
            _bPlayer->setIfChanged(nullptr);
            _bPlayerIndex->setIfChanged(-1);
        }

        void FilesModel::reload()
        {
            auto context = _context.lock();
            auto player = _player->get();
            if (context && player)
            {
                const std::size_t index = _players->indexOf(player);
                const file::Path path = player->getPath();
                auto timeline = timeline::Timeline::create(context, path);
                player = timeline::Player::create(context, timeline);
                _players->setItem(index, player);
                _player->setIfChanged(player);
                if (auto bPlayer = _bPlayer->get())
                {
                    player->setCompare({ bPlayer->getTimeline() });
                }
            }
        }

        void FilesModel::setCurrent(int value)
        {
            if (value >= 0 && value < _players->getSize())
            {
                auto player = _player->get();
                _player->setIfChanged(_players->getItem(value));
                _playerIndex->setIfChanged(value);
                if (player)
                {
                    player->setCompare({});
                }
                player = _player->get();
                auto bPlayer = _bPlayer->get();
                if (player && bPlayer)
                {
                    player->setCompare({ bPlayer->getTimeline() });
                }
            }
        }

        void FilesModel::next()
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

        void FilesModel::prev()
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

        std::shared_ptr<ftk::IObservableList<std::shared_ptr<timeline::Player> > > FilesModel::observePlayers() const
        {
            return _players;
        }

        std::shared_ptr<ftk::IObservableValue<std::shared_ptr<timeline::Player> > > FilesModel::observePlayer() const
        {
            return _player;
        }

        std::shared_ptr<ftk::IObservableValue<int> > FilesModel::observePlayerIndex() const
        {
            return _playerIndex;
        }

        void FilesModel::setB(int index)
        {
            if (auto player = _player->get())
            {
                if (index >= 0 && index < _players->getSize())
                {
                    auto bPlayer = _players->getItem(index);
                    player->setCompare({ bPlayer->getTimeline() });
                    _bPlayer->setIfChanged(bPlayer);
                    _bPlayerIndex->setIfChanged(index);
                }
                else
                {
                    player->setCompare({});
                    _bPlayer->setIfChanged(nullptr);
                    _bPlayerIndex->setIfChanged(-1);
                }
            }
        }

        void FilesModel::setCompare(timeline::Compare value)
        {
            _compare->setIfChanged(value);
        }

        std::shared_ptr<ftk::IObservableValue<std::shared_ptr<timeline::Player> > > FilesModel::observeBPlayer() const
        {
            return _bPlayer;
        }

        std::shared_ptr<ftk::IObservableValue<int> > FilesModel::observeBPlayerIndex() const
        {
            return _bPlayerIndex;
        }

        std::shared_ptr<ftk::IObservableValue<timeline::Compare> > FilesModel::observeCompare() const
        {
            return _compare;
        }

        void FilesModel::tick()
        {
            for (const auto& player : _players->get())
            {
                player->tick();
            }
        }
    }
}

