// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/AudioActions.h>

#include <tlPlayApp/Models/AudioModel.h>
#include <tlPlayApp/App.h>

namespace tl
{
    namespace play
    {
        struct AudioActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void AudioActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["VolumeUp"] = std::make_shared<dtk::Action>(
                "Volume Up",
                dtk::Key::Period,
                0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeUp();
                    }
                });

            p.actions["VolumeDown"] = std::make_shared<dtk::Action>(
                "Volume Down",
                dtk::Key::Comma,
                0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeDown();
                    }
                });

            p.actions["Mute"] = std::make_shared<dtk::Action>(
                "Mute",
                "Mute",
                dtk::Key::M,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setMute(value);
                    }
                });
        }

        AudioActions::AudioActions() :
            _p(new Private)
        {}

        AudioActions::~AudioActions()
        {}

        std::shared_ptr<AudioActions> AudioActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<AudioActions>(new AudioActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& AudioActions::getActions() const
        {
            return _p->actions;
        }
    }
}
