// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace timelineui
    {
        class TimelineViewport;
        class TimelineWidget;
    }

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
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            MainWindow();

        public:
            ~MainWindow();

            //! Create a new main window.
            static std::shared_ptr<MainWindow> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the timeline viewport.
            const std::shared_ptr<timelineui::TimelineViewport>& getTimelineViewport() const;

            //! Get the timeline widget.
            const std::shared_ptr<timelineui::TimelineWidget>& getTimelineWidget() const;

            //! Focus the current frame widget.
            void focusCurrentFrame();

            void setGeometry(const math::BBox2i&) override;
            void keyPressEvent(ui::KeyEvent&) override;
            void keyReleaseEvent(ui::KeyEvent&) override;

        private:
            void _setPlayers(const std::vector<std::shared_ptr<timeline::Player> >&);
            void _showSpeedPopup();
            void _showAudioPopup();
            void _viewportUpdate();
            void _statusUpdate(const std::vector<log::Item>&);
            void _infoUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
