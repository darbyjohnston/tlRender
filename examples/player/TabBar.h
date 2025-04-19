// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimeEdit.h>
#include <tlTimelineUI/TimeLabel.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/TabBar.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;

            //! Tab bar.
            class TabBar : public dtk::IWidget
            {
                DTK_NON_COPYABLE(TabBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<IWidget>& parent);

                TabBar() = default;

            public:
                ~TabBar();

                static std::shared_ptr<TabBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const dtk::SizeHintEvent&) override;

            private:
                std::shared_ptr<dtk::TabBar> _tabBar;
                std::shared_ptr<dtk::ListObserver<std::shared_ptr<timeline::Player> > > _playersObserver;
                std::shared_ptr<dtk::ValueObserver<int> > _playerIndexObserver;
            };
        }
    }
}
