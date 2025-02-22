// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/LogSystem.h>

#include <dtk/ui/IWidget.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Status bar widget.
        class StatusBar : public dtk::IWidget
        {
            DTK_NON_COPYABLE(StatusBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            StatusBar();

        public:
            virtual ~StatusBar();

            //! Create a new widget.
            static std::shared_ptr<StatusBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void mousePressEvent(dtk::MouseClickEvent&) override;
            void mouseReleaseEvent(dtk::MouseClickEvent&) override;

        private:
            void _logUpdate(const std::vector<dtk::LogItem>&);

            DTK_PRIVATE();
        };
    }
}
