// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGLApp/IApp.h>

struct GLFWwindow;

namespace tl
{
    //! Examples
    namespace examples
    {
        //! Example GL widgets application.
        namespace widgets_gl
        {
            class MainWindow;

            //! Application.
            class App : public gl::IApp
            {
                TLRENDER_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);

                App();

            public:
                virtual ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    const std::vector<std::string>&,
                    const std::shared_ptr<system::Context>&);
            };
        }
    }
}
