// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/Player.h>

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

                void open();
                void close();
                void reload();

                std::shared_ptr<dtk::IObservableValue<std::shared_ptr<timeline::Player> > > observePlayer() const;

            protected:
                void _tick() override;

            private:
                void _open(const std::string&);

                std::string _fileName;
                std::shared_ptr<dtk::ObservableValue<std::shared_ptr<timeline::Player> > > _player;
                std::shared_ptr<MainWindow> _window;
            };
        }
    }
}
