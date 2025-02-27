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

            //! Get the color picker.
            const dtk::Color4F& getColorPicker() const;

            //! Observe the color picker.
            std::shared_ptr<dtk::IObservableValue<dtk::Color4F> > observeColorPicker() const;

            //! Get whether the HUD is enabled.
            bool hasHUD() const;

            //! Observe whether the HUD is enabled.
            std::shared_ptr<dtk::IObservableValue<bool> > observeHUD() const;

            //! Set whether the HUD is enabled.
            void setHUD(bool);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void mouseMoveEvent(dtk::MouseMoveEvent&) override;
            void mousePressEvent(dtk::MouseClickEvent&) override;
            void mouseReleaseEvent(dtk::MouseClickEvent&) override;

        private:
            void _hudUpdate();

            DTK_PRIVATE();
        };
    }
}

