// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/Player.h>

#include <dtk/ui/App.h>
#include <dtk/ui/MainWindow.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            //! Window.
            class MainWindow : public dtk::MainWindow
            {
                DTK_NON_COPYABLE(MainWindow);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<dtk::App>&);

                MainWindow() = default;

            public:
                ~MainWindow();

                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<dtk::App>&);

            protected:
                std::shared_ptr<dtk::IRender> _createRender(const std::shared_ptr<dtk::Context>&) override;
            };

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

            protected:
                void _tick() override;

            private:
                std::string _fileName;
                std::shared_ptr<timeline::Player> _player;
                std::shared_ptr<MainWindow> _window;
            };
        }
    }
}
