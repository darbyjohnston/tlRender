// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimeEdit.h>
#include <tlTimelineUI/TimeLabel.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/Action.h>
#include <dtk/ui/RowLayout.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Playback tool bar.
        class PlaybackBar : public dtk::IWidget
        {
            DTK_NON_COPYABLE(PlaybackBar);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            PlaybackBar() = default;

        public:
            ~PlaybackBar();

            static std::shared_ptr<PlaybackBar> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;

        private:
            std::shared_ptr<timeline::Player> _player;
            std::shared_ptr<dtk::HorizontalLayout> _layout;
            std::shared_ptr<timelineui::TimeEdit> _currentTimeEdit;
            std::shared_ptr<timelineui::TimeLabel> _durationLabel;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
            std::shared_ptr<dtk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
        };
    }
}