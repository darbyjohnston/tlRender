// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <dtk/ui/Window.h>

namespace tl
{
    namespace timelineui
    {
        class TimelineWidget;
    }

    namespace play
    {
        class Viewport;
    }

    namespace play_app
    {
        class App;

        //! Window options.
        struct WindowOptions
        {
            bool  fileToolBar    = true;
            bool  compareToolBar = true;
            bool  windowToolBar  = true;
            bool  viewToolBar    = true;
            bool  toolsToolBar   = true;
            bool  timeline       = true;
            bool  bottomToolBar  = true;
            bool  statusToolBar  = true;
            float splitter       = .7F;
            float splitter2      = .7F;

            bool operator == (const WindowOptions&) const;
            bool operator != (const WindowOptions&) const;
        };

        //! Main window.
        class MainWindow : public dtk::Window
        {
            DTK_NON_COPYABLE(MainWindow);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const dtk::Size2I&);

            MainWindow();

        public:
            ~MainWindow();

            //! Create a new main window.
            static std::shared_ptr<MainWindow> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const dtk::Size2I&);

            //! Get the viewport.
            const std::shared_ptr<play::Viewport>& getViewport() const;

            //! Get the timeline widget.
            const std::shared_ptr<timelineui::TimelineWidget>& getTimelineWidget() const;

            //! Focus the current frame widget.
            void focusCurrentFrame();

            //! Get the window options.
            const WindowOptions& getWindowOptions() const;

            //! Observe the window options.
            std::shared_ptr<dtk::IObservableValue<WindowOptions> > observeWindowOptions() const;

            //! Set the window options.
            void setWindowOptions(const WindowOptions&);

            void setGeometry(const dtk::Box2I&) override;
            void keyPressEvent(dtk::KeyEvent&) override;
            void keyReleaseEvent(dtk::KeyEvent&) override;

        protected:
            std::shared_ptr<dtk::IRender> _createRender(const std::shared_ptr<dtk::Context>&) override;

            void _drop(const std::vector<std::string>&) override;

        private:
            void _playerUpdate(const std::shared_ptr<timeline::Player>&);
            void _showSpeedPopup();
            void _showAudioPopup();
            void _windowOptionsUpdate();
            void _infoUpdate();

            DTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const WindowOptions&);

        void from_json(const nlohmann::json&, WindowOptions&);

        ///@}
    }
}
