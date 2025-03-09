// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/AudioActions.h>

#include <tlPlayApp/Models/AudioModel.h>
#include <tlPlayApp/App.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct AudioActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void AudioActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["VolumeUp"] = dtk::Action::create(
                "Volume Up",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeUp();
                    }
                });

            p.actions["VolumeDown"] = dtk::Action::create(
                "Volume Down",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->volumeDown();
                    }
                });

            p.actions["Mute"] = dtk::Action::create(
                "Mute",
                "Mute",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setMute(value);
                    }
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
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

        void AudioActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            const std::map<std::string, std::string> tooltips =
            {
                {
                    "VolumeUp",
                    "Increase the audio volume.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "VolumeDown",
                    "Decrease the audio volume.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "Mute",
                    "Toggle the autio mute.\n"
                    "\n"
                    "Shortcut: {0}"
                },
            };
            for (const auto& i : p.actions)
            {
                auto j = value.shortcuts.find(dtk::Format("Audio/{0}").arg(i.first));
                if (j != value.shortcuts.end())
                {
                    i.second->setShortcut(j->second.key);
                    i.second->setShortcutModifiers(j->second.modifiers);
                    const auto k = tooltips.find(i.first);
                    if (k != tooltips.end())
                    {
                        i.second->setTooltip(dtk::Format(k->second).
                            arg(dtk::getShortcutLabel(j->second.key, j->second.modifiers)));
                    }
                }
            }
        }
    }
}
