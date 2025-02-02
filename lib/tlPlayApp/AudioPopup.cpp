// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioPopup.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/GridLayout.h>
#include <tlUI/IntEditSlider.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

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

            std::shared_ptr<ui::IntEditSlider> volumeSlider;
            std::shared_ptr<ui::ComboBox> deviceComboBox;
            std::vector<std::shared_ptr<ui::CheckBox> > channelMuteCheckBoxes;
            std::shared_ptr<ui::ButtonGroup> channelMuteButtonGroup;
            std::shared_ptr<ui::GridLayout> layout;
            std::shared_ptr<ui::HorizontalLayout> channelMuteLayout;

            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ListObserver<audio::DeviceID> > devicesObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceID> > deviceObserver;
            std::shared_ptr<dtk::ListObserver<bool> > channelMuteObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::Context>& context,
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

            p.channelMuteButtonGroup = ui::ButtonGroup::create(ui::ButtonGroupType::Toggle, context);

            p.layout = ui::GridLayout::create(context);
            p.layout->setMarginRole(ui::SizeRole::MarginInside);
            p.layout->setSpacingRole(ui::SizeRole::SpacingTool);
            auto label = ui::Label::create("Volume: ", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.volumeSlider->setParent(p.layout);
            p.layout->setGridPos(p.volumeSlider, 0, 1);
            label = ui::Label::create("Device: ", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.deviceComboBox->setParent(p.layout);
            p.layout->setGridPos(p.deviceComboBox, 1, 1);
            label = ui::Label::create("Channel mute: ", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.channelMuteLayout = ui::HorizontalLayout::create(context, p.layout);
            p.channelMuteLayout->setSpacingRole(ui::SizeRole::SpacingTool);
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
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioPopup>(new AudioPopup);
            out->_init(app, context, parent);
            return out;
        }

        void AudioPopup::_widgetUpdate()
        {
            TLRENDER_P();
            if (p.channelMuteCheckBoxes.size() != p.info.channelCount)
            {
                for (const auto& checkBox : p.channelMuteCheckBoxes)
                {
                    checkBox->setParent(nullptr);
                }
                p.channelMuteCheckBoxes.clear();
                p.channelMuteButtonGroup->clearButtons();
                if (auto context = _context.lock())
                {
                    for (size_t i = 0; i < p.info.channelCount; ++i)
                    {
                        auto checkBox = ui::CheckBox::create(
                            dtk::Format("{0}").arg(i),
                            context,
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
