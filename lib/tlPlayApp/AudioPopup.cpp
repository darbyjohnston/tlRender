// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioPopup.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

#include <dtk/ui/ButtonGroup.h>
#include <dtk/ui/CheckBox.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/GridLayout.h>
#include <dtk/ui/IntEditSlider.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace play_app
    {
        struct AudioPopup::Private
        {
            std::vector<audio::DeviceID> devices;
            std::vector<bool> channelMute;
            audio::Info info;

            std::shared_ptr<dtk::IntEditSlider> volumeSlider;
            std::shared_ptr<dtk::ComboBox> deviceComboBox;
            std::vector<std::shared_ptr<dtk::CheckBox> > channelMuteCheckBoxes;
            std::shared_ptr<dtk::ButtonGroup> channelMuteButtonGroup;
            std::shared_ptr<dtk::GridLayout> layout;
            std::shared_ptr<dtk::HorizontalLayout> channelMuteLayout;

            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ListObserver<audio::DeviceID> > devicesObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceID> > deviceObserver;
            std::shared_ptr<dtk::ListObserver<bool> > channelMuteObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidgetPopup::_init(
                context,
                "tl::play_app::AudioPopup",
                parent);
            DTK_P();

            p.volumeSlider = dtk::IntEditSlider::create(context);
            p.volumeSlider->setRange(dtk::RangeI(0, 100));
            p.volumeSlider->setStep(1);
            p.volumeSlider->setLargeStep(10);
            p.volumeSlider->setTooltip("Audio volume");

            p.deviceComboBox = dtk::ComboBox::create(context);
            p.deviceComboBox->setTooltip("Audio output device");

            p.channelMuteButtonGroup = dtk::ButtonGroup::create(context, dtk::ButtonGroupType::Toggle);

            p.layout = dtk::GridLayout::create(context);
            p.layout->setMarginRole(dtk::SizeRole::MarginInside);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingTool);
            auto label = dtk::Label::create(context, "Volume: ", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.volumeSlider->setParent(p.layout);
            p.layout->setGridPos(p.volumeSlider, 0, 1);
            label = dtk::Label::create(context, "Device: ", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.deviceComboBox->setParent(p.layout);
            p.layout->setGridPos(p.deviceComboBox, 1, 1);
            label = dtk::Label::create(context, "Channel mute: ", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.channelMuteLayout = dtk::HorizontalLayout::create(context, p.layout);
            p.channelMuteLayout->setSpacingRole(dtk::SizeRole::SpacingTool);
            p.layout->setGridPos(p.channelMuteLayout, 2, 1);
            setWidget(p.layout);

            _widgetUpdate();

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
                                0 == value ? audio::DeviceID() : _p->devices[value]);
                        }
                    }
                });

            p.channelMuteButtonGroup->setCheckedCallback(
                [this, appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        std::vector<bool> channelMute = _p->channelMute;
                        if (index >= channelMute.size())
                        {
                            channelMute.resize(index + 1);
                        }
                        channelMute[index] = value;
                        app->getAudioModel()->setChannelMute(channelMute);
                    }
                });

            p.volumeObserver = dtk::ValueObserver<float>::create(
                app->getAudioModel()->observeVolume(),
                [this](float value)
                {
                    _p->volumeSlider->setValue(std::roundf(value * 100.F));
                });

            p.devicesObserver = dtk::ListObserver<audio::DeviceID>::create(
                app->getAudioModel()->observeDevices(),
                [this](const std::vector<audio::DeviceID>& devices)
                {
                    _p->devices.clear();
                    _p->devices.push_back(audio::DeviceID());
                    _p->devices.insert(_p->devices.end(), devices.begin(), devices.end());
                    std::vector<std::string> names;
                    names.push_back("Default");
                    for (const auto& device : devices)
                    {
                        names.push_back(device.name);
                    }
                    _p->deviceComboBox->setItems(names);
                });

            p.deviceObserver = dtk::ValueObserver<audio::DeviceID>::create(
                app->getAudioModel()->observeDevice(),
                [this](const audio::DeviceID& value)
                {
                    int index = 0;
                    const auto i = std::find(_p->devices.begin(), _p->devices.end(), value);
                    if (i != _p->devices.end())
                    {
                        index = i - _p->devices.begin();
                    }
                    _p->deviceComboBox->setCurrentIndex(index);
                });

            p.channelMuteObserver = dtk::ListObserver<bool>::create(
                app->getAudioModel()->observeChannelMute(),
                [this](const std::vector<bool>& value)
                {
                    _p->channelMute = value;
                    _widgetUpdate();
                });

            p.playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _p->info = value ? value->getIOInfo().audio : audio::Info();
                    _widgetUpdate();
                });
        }

        AudioPopup::AudioPopup() :
            _p(new Private)
        {}

        AudioPopup::~AudioPopup()
        {}

        std::shared_ptr<AudioPopup> AudioPopup::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioPopup>(new AudioPopup);
            out->_init(context, app, parent);
            return out;
        }

        void AudioPopup::_widgetUpdate()
        {
            DTK_P();
            if (p.channelMuteCheckBoxes.size() != p.info.channelCount)
            {
                for (const auto& checkBox : p.channelMuteCheckBoxes)
                {
                    checkBox->setParent(nullptr);
                }
                p.channelMuteCheckBoxes.clear();
                p.channelMuteButtonGroup->clearButtons();
                if (auto context = getContext())
                {
                    for (size_t i = 0; i < p.info.channelCount; ++i)
                    {
                        auto checkBox = dtk::CheckBox::create(
                            context,
                            dtk::Format("{0}").arg(i),
                            p.channelMuteLayout);
                        p.channelMuteCheckBoxes.push_back(checkBox);
                        p.channelMuteButtonGroup->addButton(checkBox);
                    }
                }
            }
            for (size_t i = 0; i < p.channelMute.size() && i < p.channelMuteCheckBoxes.size(); ++i)
            {
                p.channelMuteCheckBoxes[i]->setChecked(p.channelMute[i]);
            }
        }
    }
}
