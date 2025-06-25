// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/ui/Label.h>
#include <feather-tk/core/Timer.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Status bar.
        class StatusBar : public feather_tk::IWidget
        {
            FEATHER_TK_NON_COPYABLE(StatusBar);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            StatusBar() = default;

        public:
            ~StatusBar();

            static std::shared_ptr<StatusBar> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const feather_tk::Box2I&) override;
            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

        private:
            void _logUpdate(const std::vector<feather_tk::LogItem>&);

            std::shared_ptr<feather_tk::Label> _logLabel;
            std::shared_ptr<feather_tk::Timer> _logTimer;
            std::shared_ptr<feather_tk::ListObserver<feather_tk::LogItem> > _logObserver;
        };
    }
}