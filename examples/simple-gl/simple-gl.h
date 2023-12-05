// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLApp/IApp.h>
#include <tlGLApp/Window.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace examples
    {
        namespace simple_gl
        {
            class App : public gl_app::IApp
            {
                TLRENDER_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);

                App();

            public:
                ~App();

                static std::shared_ptr<App> create(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);

            protected:
                void _tick() override;

            private:
                std::string _fileName;
                std::shared_ptr<timeline::Player> _player;
                std::shared_ptr<gl_app::Window> _window;
            };
        }
    }
}
