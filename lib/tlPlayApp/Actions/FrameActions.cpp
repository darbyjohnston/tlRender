// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/FrameActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

namespace tl
{
    namespace play
    {
        struct FrameActions::Private
        {
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void FrameActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            IActions::_init(context, app, "Frame");
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            _actions["Start"] = dtk::Action::create(
                "Goto Start",
                "TimeStart",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->gotoStart();
                        }
                    }
                });

            _actions["End"] = dtk::Action::create(
                "Goto End",
                "TimeEnd",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->gotoEnd();
                        }
                    }
                });

            _actions["Prev"] = dtk::Action::create(
                "Previous Frame",
                "FramePrev",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->framePrev();
                        }
                    }
                });

            _actions["PrevX10"] = dtk::Action::create(
                "Previous Frame X10",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FramePrevX10);
                        }
                    }
                });

            _actions["PrevX100"] = dtk::Action::create(
                "Previous Frame X100",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FramePrevX100);
                        }
                    }
                });

            _actions["Next"] = dtk::Action::create(
                "Next Frame",
                "FrameNext",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->frameNext();
                        }
                    }
                });

            _actions["NextX10"] = dtk::Action::create(
                "Next Frame X10",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FrameNextX10);
                        }
                    }
                });

            _actions["NextX100"] = dtk::Action::create(
                "Next Frame X100",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FrameNextX100);
                        }
                    }
                });

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            _actions["FocusCurrent"] = dtk::Action::create(
                "Focus Current Frame",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->focusCurrentFrame();
                    }
                });

            _tooltips =
            {
                { "Start", "Go to the start frame." },
                { "End", "Go to the end frame." },
                { "Prev", "Go to the previous frame." },
                { "PrevX10", "Go to the previous frame X10." },
                { "PrevX100", "Go to the previous frame X100." },
                { "Next", "Go to the next frame." },
                { "NextX10", "Go to the next frame X10." },
                { "NextX100", "Go to the next frame X100." },
                { "FocusCurrent", "Set the keyboard focus to the current frame editor." }
            };

            _shortcutsUpdate(app->getSettingsModel()->getShortcuts());

            p.playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _actions["Start"]->setEnabled(value.get());
                    _actions["End"]->setEnabled(value.get());
                    _actions["Prev"]->setEnabled(value.get());
                    _actions["PrevX10"]->setEnabled(value.get());
                    _actions["PrevX100"]->setEnabled(value.get());
                    _actions["Next"]->setEnabled(value.get());
                    _actions["NextX10"]->setEnabled(value.get());
                    _actions["NextX100"]->setEnabled(value.get());
                    _actions["FocusCurrent"]->setEnabled(value.get());
                });
        }

        FrameActions::FrameActions() :
            _p(new Private)
        {}

        FrameActions::~FrameActions()
        {}

        std::shared_ptr<FrameActions> FrameActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            auto out = std::shared_ptr<FrameActions>(new FrameActions);
            out->_init(context, app, mainWindow);
            return out;
        }
    }
}
