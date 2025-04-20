// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>
#include <tlTimeline/TimeUnits.h>

#include <dtk/ui/App.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class MainWindow;

            //! Application.
            class App : public dtk::App
            {
                DTK_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    std::vector<std::string>&);

                App();

            public:
                ~App();

                static std::shared_ptr<App> create(
                    const std::shared_ptr<dtk::Context>&,
                    std::vector<std::string>&);

                const std::shared_ptr<timeline::TimeUnitsModel>& getTimeUnitsModel() const;

                void open();
                void close();
                void close(int);
                void closeAll();
                void reload();
                void setCurrent(int);
                void next();
                void prev();

                std::shared_ptr<dtk::IObservableList<std::shared_ptr<timeline::Player> > > observePlayers() const;
                std::shared_ptr<dtk::IObservableValue<std::shared_ptr<timeline::Player> > > observePlayer() const;
                std::shared_ptr<dtk::IObservableValue<int> > observePlayerIndex() const;

                void setB(int);
                void setCompare(timeline::Compare);

                std::shared_ptr<dtk::IObservableValue<std::shared_ptr<timeline::Player> > > observeBPlayer() const;
                std::shared_ptr<dtk::IObservableValue<int> > observeBPlayerIndex() const;
                std::shared_ptr<dtk::IObservableValue<timeline::Compare> > observeCompare() const;

            protected:
                void _tick() override;

            private:
                void _open(const std::string&);

                std::vector<std::string> _fileNames;
                std::shared_ptr<timeline::TimeUnitsModel> _timeUnitsModel;
                std::shared_ptr<dtk::ObservableList<std::shared_ptr<timeline::Player> > > _players;
                std::shared_ptr<dtk::ObservableValue<std::shared_ptr<timeline::Player> > > _player;
                std::shared_ptr<dtk::ObservableValue<int> > _playerIndex;
                std::shared_ptr<dtk::ObservableValue<std::shared_ptr<timeline::Player> > > _bPlayer;
                std::shared_ptr<dtk::ObservableValue<int> > _bPlayerIndex;
                std::shared_ptr<dtk::ObservableValue<timeline::Compare> > _compare;
                std::shared_ptr<MainWindow> _window;
            };
        }
    }
}
