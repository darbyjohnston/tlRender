// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            //! Main window.
            class MainWindow : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(MainWindow);

            protected:
                void _init(
                    const std::shared_ptr<timeline::Player>&,
                    const std::shared_ptr<system::Context>&);

                MainWindow();

            public:
                ~MainWindow();

                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<timeline::Player>&,
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;

            private:
                void _infoUpdate();

                TLRENDER_PRIVATE();
            };
        }
    }
}
