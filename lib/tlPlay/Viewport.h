// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace play
    {
        class Viewport : public timelineui::TimelineViewport
        {
            DTK_NON_COPYABLE(Viewport);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Viewport();

        public:
            virtual ~Viewport();

            static std::shared_ptr<Viewport> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get whether the HUD is enabled.
            bool hasHUD() const;

            //! Observe whether the HUD is enabled.
            std::shared_ptr<dtk::IObservableValue<bool> > observeHUD() const;

            //! Set whether the HUD is enabled.
            void setHUD(bool);

            void setGeometry(const dtk::Box2I&) override;
            void childRemovedEvent(const ui::ChildEvent&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void mouseMoveEvent(ui::MouseMoveEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;

        private:
            void _hudUpdate();
            void _colorPickersUpdate();
            void _colorWidgetsUpdate();

            DTK_PRIVATE();
        };
    }
}

