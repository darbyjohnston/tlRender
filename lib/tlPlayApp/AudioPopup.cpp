// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioPopup.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

#include <tlUI/ComboBox.h>
#include <tlUI/IntEditSlider.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct AudioPopup::Private
        {
            std::vector<std::string> devices;

            std::shared_ptr<ui::IntEditSlider> volumeSlider;
            std::shared_ptr<ui::ComboBox> deviceComboBox;
            std::shared_ptr<ui::VerticalLayout> layout;

            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ListObserver<std::string> > devicesObserver;
            std::shared_ptr<observer::ValueObserver<std::string> > deviceObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidgetPopup::_init(
                "tl::play_app::AudioPopup",
                context,
                parent);
            TLRENDER_P();

            p.volumeSlider = ui::IntEditSlider::create(context);
            p.volumeSlider->setRange(math::IntRange(0, 100));
            p.volumeSlider->setStep(1);
            p.volumeSlider->setLargeStep(10);
            p.volumeSlider->setToolTip("Audio volume");

            p.deviceComboBox = ui::ComboBox::create(context);
            p.deviceComboBox->setToolTip("Audio output device");

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setMarginRole(ui::SizeRole::MarginInside);
            p.layout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.volumeSlider->setParent(p.layout);
            p.deviceComboBox->setParent(p.layout);
            setWidget(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.volumeSlider->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setVolume(value / 100.F);
                    }
                });

            p.deviceComboBox->setIndexCallback(
                [this, appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (value >= 0 && value < _p->devices.size())
                        {
                            app->getAudioModel()->setDevice(
                                0 == value ? std::string() : _p->devices[value]);
                        }
                    }
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                app->getAudioModel()->observeVolume(),
                [this](float value)
                {
                    _p->volumeSlider->setValue(std::roundf(value * 100.F));
                });

            p.devicesObserver = observer::ListObserver<std::string>::create(
                app->getAudioModel()->observeDevices(),
                [this](const std::vector<std::string>& value)
                {
                    _p->devices.clear();
                    _p->devices.push_back("Default");
                    _p->devices.insert(_p->devices.end(), value.begin(), value.end());
                    _p->deviceComboBox->setItems(_p->devices);
                });

            p.deviceObserver = observer::ValueObserver<std::string>::create(
                app->getAudioModel()->observeDevice(),
                [this](const std::string& value)
                {
                    int index = 0;
                    const auto i = std::find(_p->devices.begin(), _p->devices.end(), value);
                    if (i != _p->devices.end())
                    {
                        index = i - _p->devices.begin();
                    }
                    _p->deviceComboBox->setCurrentIndex(index);
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
