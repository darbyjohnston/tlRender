// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimeEdit.h>
#include <tlTimelineUI/TimeLabel.h>

#include <tlTimeline/Player.h>
#include <tlTimeline/TimeUnits.h>

#include <feather-tk/ui/Action.h>
#include <feather-tk/ui/ComboBox.h>
#include <feather-tk/ui/RowLayout.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Playback tool bar.
        class PlaybackBar : public feather_tk::IWidget
        {
            FEATHER_TK_NON_COPYABLE(PlaybackBar);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<feather_tk::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            PlaybackBar() = default;

        public:
            ~PlaybackBar();

            static std::shared_ptr<PlaybackBar> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<feather_tk::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const feather_tk::Box2I&) override;
            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

        private:
            std::shared_ptr<timeline::Player> _player;
            std::shared_ptr<feather_tk::HorizontalLayout> _layout;
            std::shared_ptr<timelineui::TimeEdit> _currentTimeEdit;
            std::shared_ptr<timelineui::TimeLabel> _durationLabel;
            std::shared_ptr<feather_tk::ComboBox> _timeUnitsComboBox;
            std::shared_ptr<feather_tk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > _currentTimeObserver;
            std::shared_ptr<feather_tk::ValueObserver<timeline::TimeUnits> > _timeUnitsObserver;
        };
    }
}
