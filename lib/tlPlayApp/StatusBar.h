// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace play_app
    {
        //! Status bar widget.
        class StatusBar : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(StatusBar);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            StatusBar();

        public:
            virtual ~StatusBar();

            //! Create a new widget.
            static std::shared_ptr<StatusBar> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void mousePressEvent(ui::MouseClickEvent&) override;
            void mouseReleaseEvent(ui::MouseClickEvent&) override;

        private:
            void _widgetUpdate(const std::vector<log::Item>&);

            TLRENDER_PRIVATE();
        };
    }
}
