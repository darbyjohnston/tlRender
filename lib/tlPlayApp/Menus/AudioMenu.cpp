// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/AudioMenu.h>

#include <tlPlayApp/Models/AudioModel.h>
#include <tlPlayApp/App.h>

namespace tl
{
    namespace play
    {
        struct AudioMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
        };

        void AudioMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.actions = actions;

            addItem(p.actions["VolumeUp"]);
            addItem(p.actions["VolumeDown"]);
            addItem(p.actions["Mute"]);

            p.volumeObserver = dtk::ValueObserver<float>::create(
                app->getAudioModel()->observeVolume(),
                [this](float value)
                {
                    setItemEnabled(_p->actions["VolumeUp"], value < 1.F);
                    setItemEnabled(_p->actions["VolumeDown"], value > 0.F);
                });

            p.muteObserver = dtk::ValueObserver<bool>::create(
                app->getAudioModel()->observeMute(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["Mute"], value);
                });
        }

        AudioMenu::AudioMenu() :
            _p(new Private)
        {}

        AudioMenu::~AudioMenu()
        {}

        std::shared_ptr<AudioMenu> AudioMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioMenu>(new AudioMenu);
            out->_init(context, app, actions, parent);
            return out;
        }
    }
}
