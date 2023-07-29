// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/AudioPopup.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Settings.h>

#include <tlUI/IntEditSlider.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        struct AudioPopup::Private
        {
            std::shared_ptr<ui::ToolButton> muteButton;
            std::shared_ptr<ui::IntEditSlider> volumeSlider;
            std::shared_ptr<ui::HorizontalLayout> layout;
            std::shared_ptr<observer::ValueObserver<int> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IMenuPopup::_init(
                "tl::play_gl::AudioPopup",
                context,
                parent);
            TLRENDER_P();

            p.muteButton = ui::ToolButton::create(context);
            p.muteButton->setCheckable(true);
            p.muteButton->setIcon("Mute");

            p.volumeSlider = ui::IntEditSlider::create(context);
            p.volumeSlider->getModel()->setRange(math::IntRange(0, 100));
            p.volumeSlider->getModel()->setStep(1);
            p.volumeSlider->getModel()->setLargeStep(10);

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginInside);
            p.layout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.muteButton->setParent(p.layout);
            p.volumeSlider->setParent(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.muteButton->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Audio/Mute", value);
                    }
                });

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            float value = 0.F;
                            settings->getValue("Audio/Volume", value);
                            p.volumeSlider->getModel()->setValue(value * 100.F);
                        }
                        {
                            bool value = false;
                            settings->getValue("Audio/Mute", value);
                            p.muteButton->setChecked(value);
                        }
                    }
                });

            p.volumeObserver = observer::ValueObserver<int>::create(
                p.volumeSlider->getModel()->observeValue(),
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Audio/Volume", value / 100.F);
                    }
                });
        }

        AudioPopup::AudioPopup() :
            _p(new Private)
        {}

        AudioPopup::~AudioPopup()
        {}

        std::shared_ptr<AudioPopup> AudioPopup::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioPopup>(new AudioPopup);
            out->_init(app, context, parent);
            return out;
        }
    }
}
