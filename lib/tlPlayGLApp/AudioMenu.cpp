// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/AudioMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct AudioMenu::Private
        {
        };

        void AudioMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            auto item = std::make_shared<ui::MenuItem>(
                "Increase Volume",
                ui::Key::Period,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Decrease Volume",
                ui::Key::Comma,
                0,
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Mute",
                "Mute",
                ui::Key::M,
                0,
                [this](bool value)
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);
        }

        AudioMenu::AudioMenu() :
            _p(new Private)
        {}

        AudioMenu::~AudioMenu()
        {}

        std::shared_ptr<AudioMenu> AudioMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<AudioMenu>(new AudioMenu);
            out->_init(app, context);
            return out;
        }
    }
}
