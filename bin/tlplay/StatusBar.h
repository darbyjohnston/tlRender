// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Label.h>
#include <dtk/core/Timer.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Status bar.
        class StatusBar : public dtk::IWidget
        {
            DTK_NON_COPYABLE(StatusBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            StatusBar() = default;

        public:
            ~StatusBar();

            static std::shared_ptr<StatusBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            void _logUpdate(const std::vector<dtk::LogItem>&);

            std::shared_ptr<dtk::Label> _logLabel;
            std::shared_ptr<dtk::Timer> _logTimer;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > _logObserver;
        };
    }
}