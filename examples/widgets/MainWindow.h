// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUIApp/Window.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            //! Main window.
            class MainWindow : public ui_app::Window
            {
                TLRENDER_NON_COPYABLE(MainWindow);

            protected:
                void _init(const std::shared_ptr<dtk::Context>&);

                MainWindow();

            public:
                ~MainWindow();

                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<dtk::Context>&);

                void setGeometry(const math::Box2i&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
