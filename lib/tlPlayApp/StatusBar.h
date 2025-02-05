// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

#include <dtk/core/LogSystem.h>

namespace tl
{
    namespace play_app
    {
        //! Status bar widget.
        class StatusBar : public ui::IWidget
        {
            DTK_NON_COPYABLE(StatusBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            StatusBar();

        public:
            virtual ~StatusBar();

            //! Create a new widget.
            static std::shared_ptr<StatusBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;

        private:
            void _widgetUpdate(const std::vector<dtk::LogItem>&);

            DTK_PRIVATE();
        };
    }
}
