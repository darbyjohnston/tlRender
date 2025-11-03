// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTimeline/Player.h>

#include <ftk/UI/Label.h>
#include <ftk/UI/RowLayout.h>
#include <ftk/Core/Timer.h>

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
            void _infoUpdate(const std::shared_ptr<timeline::Player>&);

            std::shared_ptr<ftk::HorizontalLayout> _layout;
            std::map<std::string, std::shared_ptr<ftk::Label> > _labels;
            std::shared_ptr<ftk::Timer> _logTimer;
            std::shared_ptr<ftk::ListObserver<ftk::LogItem> > _logObserver;
            std::shared_ptr<ftk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
        };
    }
}