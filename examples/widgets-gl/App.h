// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGLApp/IApp.h>

struct GLFWwindow;

namespace tl
{
    namespace examples
    {
        //! Example GL widgtes application.
        namespace widgets_gl
        {
            class MainWindow;

            //! Application.
            class App : public gl::IApp
            {
                TLRENDER_NON_COPYABLE(App);

            protected:
                void _init(
                    int argc,
                    char* argv[],
                    const std::shared_ptr<system::Context>&);

                App();

            public:
                ~App() override;

                //! Create a new application.
                static std::shared_ptr<App> create(
                    int argc,
                    char* argv[],
                    const std::shared_ptr<system::Context>&);

            private:
                std::shared_ptr<MainWindow> _mainWindow;
            };
        }
    }
}
