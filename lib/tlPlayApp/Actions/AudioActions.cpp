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
            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
        };

        void AudioActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            IActions::_init(context, app, "Audio");
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            _actions["VolumeUp"] = dtk::Action::create(
                "Volume Up",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeUp();
                    }
                });

            _actions["VolumeDown"] = dtk::Action::create(
                "Volume Down",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeDown();
                    }
                });

            _actions["Mute"] = dtk::Action::create(
                "Mute",
                "Mute",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setMute(value);
                    }
                });

            _tooltips =
            {
                { "VolumeUp", "Increase the audio volume." },
                { "VolumeDown", "Decrease the audio volume." },
                { "Mute", "Toggle the autio mute." },
            };

            _shortcutsUpdate(app->getSettingsModel()->getShortcuts());

            p.volumeObserver = dtk::ValueObserver<float>::create(
                app->getAudioModel()->observeVolume(),
                [this](float value)
                {
                    _actions["VolumeUp"]->setEnabled(value < 1.F);
                    _actions["VolumeDown"]->setEnabled(value > 0.F);
                });

            p.muteObserver = dtk::ValueObserver<bool>::create(
                app->getAudioModel()->observeMute(),
                [this](bool value)
                {
                    _actions["Mute"]->setChecked(value);
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
    }
}
