// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUIApp/App.h>

namespace tl
{
    //! Examples
    namespace examples
    {
        //! Example widgets application.
        namespace widgets
        {
            class MainWindow;

            //! Application.
            class App : public ui_app::App
            {
                DTK_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::vector<std::string>&);

                App();

            public:
                virtual ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::vector<std::string>&);
            };
        }
    }
}
