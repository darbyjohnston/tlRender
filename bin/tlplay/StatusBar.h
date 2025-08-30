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
        class StatusBar : public ftk::IWidget
        {
            FTK_NON_COPYABLE(StatusBar);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            StatusBar() = default;

        public:
            ~StatusBar();

            static std::shared_ptr<StatusBar> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            void _logUpdate(const std::vector<ftk::LogItem>&);

            std::shared_ptr<ftk::Label> _logLabel;
            std::shared_ptr<ftk::Timer> _logTimer;
            std::shared_ptr<ftk::ListObserver<ftk::LogItem> > _logObserver;
        };
    }
}