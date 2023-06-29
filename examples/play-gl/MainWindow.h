// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace examples
    {
        namespace play_gl
        {
            class App;

            //! Main window.
            class MainWindow : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(MainWindow);

            protected:
                void _init(
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<system::Context>&);

                MainWindow();

            public:
                ~MainWindow();

                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;
                void keyPressEvent(ui::KeyEvent&) override;
                void keyReleaseEvent(ui::KeyEvent&) override;

            private:
                void _setPlayer(const std::shared_ptr<timeline::Player>&);
                void _playbackUpdate();
                void _infoUpdate();

                TLRENDER_PRIVATE();
            };
        }
    }
}
