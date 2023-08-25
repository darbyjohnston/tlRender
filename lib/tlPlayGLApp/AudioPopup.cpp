// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/AudioPopup.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Settings.h>

#include <tlPlay/AudioModel.h>

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

            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidgetPopup::_init(
                "tl::play_gl::AudioPopup",
                context,
                parent);
            TLRENDER_P();

            p.muteButton = ui::ToolButton::create(context);
            p.muteButton->setCheckable(true);
            p.muteButton->setIcon("Mute");
            p.muteButton->setToolTip("Mute the audio");

            p.volumeSlider = ui::IntEditSlider::create(context);
            p.volumeSlider->setRange(math::IntRange(0, 100));
            p.volumeSlider->setStep(1);
            p.volumeSlider->setLargeStep(10);
            p.volumeSlider->setToolTip("Audio volume");

            p.layout = ui::HorizontalLayout::create(context);
            p.layout->setMarginRole(ui::SizeRole::MarginInside);
            p.layout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.muteButton->setParent(p.layout);
            p.volumeSlider->setParent(p.layout);
            setWidget(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.muteButton->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setMute(value);
                    }
                });

            p.volumeSlider->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setVolume(value / 100.F);
                    }
                });

            p.muteObserver = observer::ValueObserver<bool>::create(
                app->getAudioModel()->observeMute(),
                [this](bool value)
                {
                    _p->muteButton->setChecked(value);
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                app->getAudioModel()->observeVolume(),
                [this](float value)
                {
                    _p->volumeSlider->setValue(std::roundf(value * 100.F));
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
