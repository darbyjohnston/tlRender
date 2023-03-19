// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLFWApp/IApp.h>

struct GLFWwindow;

namespace tl
{
    namespace examples
    {
        //! Example GLFW user interface application.
        namespace ui_glfw
        {
            class MainWindow;

            //! Application.
            class App : public glfw::IApp
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
