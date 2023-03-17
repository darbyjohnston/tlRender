// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Window.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            //! Main window.
            class MainWindow : public ui::Window
            {
                TLRENDER_NON_COPYABLE(MainWindow);

            protected:
                void _init(const std::shared_ptr<system::Context>&);

                MainWindow();

            public:
                ~MainWindow();

                //! Create a new main window.
                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
