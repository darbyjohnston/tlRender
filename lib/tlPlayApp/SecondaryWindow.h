// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/Window.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/Window.h>

namespace tl
{
    namespace timelineui
    {
        class TimelineViewport;
    }

    namespace play_app
    {
        class App;

        //! Secondary window.
        class SecondaryWindow : public timelineui::Window
        {
            DTK_NON_COPYABLE(SecondaryWindow);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<dtk::Window>& shared);

            SecondaryWindow();

        public:
            virtual ~SecondaryWindow();

            static std::shared_ptr<SecondaryWindow> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<dtk::Window>& shared = nullptr);

            //! Get the viewport.
            const std::shared_ptr<timelineui::TimelineViewport>& getViewport() const;

            //! Set the view.
            void setView(
                const dtk::V2I& pos,
                double          zoom,
                bool            frame);

        private:
            DTK_PRIVATE();
        };
    }
}
