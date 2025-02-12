// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/DevicesTool.h>

#include <tlPlayApp/App.h>

#if defined(TLRENDER_BMD)
#include <tlPlay/BMDDevicesModel.h>
#endif // TLRENDER_BMD

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <dtk/ui/Bellows.h>
#include <dtk/ui/CheckBox.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/Divider.h>
#include <dtk/ui/FloatEdit.h>
#include <dtk/ui/FloatEditSlider.h>
#include <dtk/ui/GridLayout.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>

#include <sstream>

namespace tl
{
    namespace play_app
    {
        struct DevicesTool::Private
        {
#if defined(TLRENDER_BMD)
            std::shared_ptr<dtk::CheckBox> enabledCheckBox;
            std::shared_ptr<dtk::ComboBox> deviceComboBox;
            std::shared_ptr<dtk::ComboBox> displayModeComboBox;
            std::shared_ptr<dtk::ComboBox> pixelTypeComboBox;
            std::shared_ptr<dtk::CheckBox> _444SDIVideoOutputCheckBox;
            std::shared_ptr<dtk::ComboBox> videoLevelsComboBox;
            std::shared_ptr<dtk::ComboBox> hdrModeComboBox;
            std::vector<std::pair< std::shared_ptr<dtk::FloatEdit>, std::shared_ptr<dtk::FloatEdit> > > primariesFloatEdits;
            std::pair< std::shared_ptr<dtk::FloatEdit>, std::shared_ptr<dtk::FloatEdit> > masteringLuminanceFloatEdits;
            std::shared_ptr<dtk::FloatEditSlider> maxCLLSlider;
            std::shared_ptr<dtk::FloatEditSlider> maxFALLSlider;

            std::shared_ptr<dtk::ValueObserver<bmd::DevicesModelData> > dataObserver;
#endif // TLRENDER_BMD
        };

        void DevicesTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Devices,
                "tl::play_app::DevicesTool",
                parent);
            DTK_P();

#if defined(TLRENDER_BMD)
            p.enabledCheckBox = dtk::CheckBox::create(context);

            p.deviceComboBox = dtk::ComboBox::create(context);
            p.deviceComboBox->setHStretch(dtk::Stretch::Expanding);
            p.displayModeComboBox = dtk::ComboBox::create(context);
            p.displayModeComboBox->setHStretch(dtk::Stretch::Expanding);
            p.pixelTypeComboBox = dtk::ComboBox::create(context);
            p.pixelTypeComboBox->setHStretch(dtk::Stretch::Expanding);

            p._444SDIVideoOutputCheckBox = dtk::CheckBox::create(context);

            p.videoLevelsComboBox = dtk::ComboBox::create(context, dtk::getVideoLevelsLabels());
            p.videoLevelsComboBox->setHStretch(dtk::Stretch::Expanding);

            p.hdrModeComboBox = dtk::ComboBox::create(context, bmd::getHDRModeLabels());
            p.hdrModeComboBox->setHStretch(dtk::Stretch::Expanding);

            for (size_t i = 0; i < static_cast<size_t>(image::HDRPrimaries::Count); ++i)
            {
                auto min = dtk::FloatEdit::create(context);
                min->setRange(dtk::RangeF(0.F, 1.F));
                min->setStep(.01F);
                min->setLargeStep(.1F);
                auto max = dtk::FloatEdit::create(context);
                max->setRange(dtk::RangeF(0.F, 1.F));
                max->setStep(.01F);
                max->setLargeStep(.1F);
                p.primariesFloatEdits.push_back(std::make_pair(min, max));
            }

            p.masteringLuminanceFloatEdits.first = dtk::FloatEdit::create(context);
            p.masteringLuminanceFloatEdits.first->setRange(dtk::RangeF(0.F, 10000.F));
            p.masteringLuminanceFloatEdits.second = dtk::FloatEdit::create(context);
            p.masteringLuminanceFloatEdits.second->setRange(dtk::RangeF(0.F, 10000.F));

            p.maxCLLSlider = dtk::FloatEditSlider::create(context);
            p.maxCLLSlider->setRange(dtk::RangeF(0.F, 10000.F));

            p.maxFALLSlider = dtk::FloatEditSlider::create(context);
            p.maxFALLSlider->setRange(dtk::RangeF(0.F, 10000.F));

            auto layout = dtk::VerticalLayout::create(context);
            layout->setSpacingRole(dtk::SizeRole::None);

            auto bellows = dtk::Bellows::create(context, "Output", layout);
            auto gridLayout = dtk::GridLayout::create(context);
            gridLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            gridLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Enabled:", gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.enabledCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p.enabledCheckBox, 0, 1);
            label = dtk::Label::create(context, "Device:", gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.deviceComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.deviceComboBox, 1, 1);
            label = dtk::Label::create(context, "Display mode:", gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.displayModeComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.displayModeComboBox, 2, 1);
            label = dtk::Label::create(context, "Pixel type:", gridLayout);
            gridLayout->setGridPos(label, 3, 0);
            p.pixelTypeComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.pixelTypeComboBox, 3, 1);
            label = dtk::Label::create(context, "444 SDI video output:", gridLayout);
            gridLayout->setGridPos(label, 4, 0);
            p._444SDIVideoOutputCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p._444SDIVideoOutputCheckBox, 4, 1);
            label = dtk::Label::create(context, "Video levels:", gridLayout);
            gridLayout->setGridPos(label, 5, 0);
            p.videoLevelsComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.videoLevelsComboBox, 5, 1);
            bellows->setWidget(gridLayout);

            bellows = dtk::Bellows::create(context, "HDR", layout);
            gridLayout = dtk::GridLayout::create(context);
            gridLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            gridLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            label = dtk::Label::create(context, "Mode:", gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.hdrModeComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.hdrModeComboBox, 0, 1);
            const std::array<std::string, static_cast<size_t>(image::HDRPrimaries::Count)> primariesLabels =
            {
                "Red primaries:",
                "Green primaries:",
                "Blue primaries:",
                "White primaries:"
            };
            for (size_t i = 0; i < static_cast<size_t>(image::HDRPrimaries::Count); ++i)
            {
                label = dtk::Label::create(context, primariesLabels[i], gridLayout);
                gridLayout->setGridPos(label, 2 + i, 0);
                auto hLayout = dtk::HorizontalLayout::create(context, gridLayout);
                hLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
                p.primariesFloatEdits[i].first->setParent(hLayout);
                p.primariesFloatEdits[i].second->setParent(hLayout);
                gridLayout->setGridPos(hLayout, 2 + i, 1);
            }
            label = dtk::Label::create(context, "Mastering luminance:", gridLayout);
            gridLayout->setGridPos(label, 7, 0);
            auto hLayout = dtk::HorizontalLayout::create(context, gridLayout);
            hLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.masteringLuminanceFloatEdits.first->setParent(hLayout);
            p.masteringLuminanceFloatEdits.second->setParent(hLayout);
            gridLayout->setGridPos(hLayout, 7, 1);
            label = dtk::Label::create(context, "Maximum CLL:", gridLayout);
            gridLayout->setGridPos(label, 8, 0);
            p.maxCLLSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.maxCLLSlider, 8, 1);
            label = dtk::Label::create(context, "Maximum FALL:", gridLayout);
            gridLayout->setGridPos(label, 9, 0);
            p.maxFALLSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.maxFALLSlider, 9, 1);
            bellows->setWidget(gridLayout);

            auto scrollWidget = dtk::ScrollWidget::create(context);
            scrollWidget->setBorder(false);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setDeviceEnabled(value);
                    }
                });

            p.deviceComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setDeviceIndex(value);
                    }
                });
            p.displayModeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setDisplayModeIndex(value);
                    }
                });
            p.pixelTypeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setPixelTypeIndex(value);
                    }
                });

            p._444SDIVideoOutputCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getBMDDevicesModel()->observeData()->get().boolOptions;
                        options[bmd::Option::_444SDIVideoOutput] = value;
                        app->getBMDDevicesModel()->setBoolOptions(options);
                    }
                });

            p.videoLevelsComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setVideoLevels(static_cast<dtk::VideoLevels>(value));
                    }
                });

            p.hdrModeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setHDRMode(static_cast<bmd::HDRMode>(value));
                    }
                });

            for (size_t i = 0; i < static_cast<size_t>(image::HDRPrimaries::Count); ++i)
            {
                p.primariesFloatEdits[i].first->setCallback(
                    [appWeak, i](float value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto hdrData = app->getBMDDevicesModel()->observeData()->get().hdrData;
                            hdrData.primaries[i].x = value;
                            app->getBMDDevicesModel()->setHDRData(hdrData);
                        }
                    });
                p.primariesFloatEdits[i].second->setCallback(
                    [appWeak, i](float value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto hdrData = app->getBMDDevicesModel()->observeData()->get().hdrData;
                            hdrData.primaries[i].y = value;
                            app->getBMDDevicesModel()->setHDRData(hdrData);
                        }
                    });
            }

            p.masteringLuminanceFloatEdits.first->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()->observeData()->get().hdrData;
                        hdrData.displayMasteringLuminance = dtk::RangeF(value, hdrData.displayMasteringLuminance.max());
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });
            p.masteringLuminanceFloatEdits.second->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()->observeData()->get().hdrData;
                        hdrData.displayMasteringLuminance = dtk::RangeF(hdrData.displayMasteringLuminance.min(), value);
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });

            p.maxCLLSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()->observeData()->get().hdrData;
                        hdrData.maxCLL = value;
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });
            p.maxFALLSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()->observeData()->get().hdrData;
                        hdrData.maxFALL = value;
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });

            p.dataObserver = dtk::ValueObserver<bmd::DevicesModelData>::create(
                app->getBMDDevicesModel()->observeData(),
                [this](const bmd::DevicesModelData& value)
                {
                    DTK_P();
                    p.enabledCheckBox->setChecked(value.deviceEnabled);

                    p.deviceComboBox->setItems(value.devices);
                    p.deviceComboBox->setCurrentIndex(value.deviceIndex);
                    p.displayModeComboBox->setItems(value.displayModes);
                    p.displayModeComboBox->setCurrentIndex(value.displayModeIndex);
                    std::vector<std::string> pixelTypes;
                    for (const auto& pixelType : value.pixelTypes)
                    {
                        std::stringstream ss;
                        ss << pixelType;
                        pixelTypes.push_back(ss.str());
                    }
                    p.pixelTypeComboBox->setItems(pixelTypes);
                    p.pixelTypeComboBox->setCurrentIndex(value.pixelTypeIndex);

                    const auto i = value.boolOptions.find(bmd::Option::_444SDIVideoOutput);
                    p._444SDIVideoOutputCheckBox->setChecked(i != value.boolOptions.end() ? i->second : false);

                    p.videoLevelsComboBox->setCurrentIndex(static_cast<int>(value.videoLevels));

                    p.hdrModeComboBox->setCurrentIndex(static_cast<int>(value.hdrMode));

                    for (size_t i = 0; i < static_cast<size_t>(image::HDRPrimaries::Count); ++i)
                    {
                        p.primariesFloatEdits[i].first->setValue(value.hdrData.primaries[i].x);
                        p.primariesFloatEdits[i].first->setEnabled(bmd::HDRMode::Custom == value.hdrMode);
                        p.primariesFloatEdits[i].second->setValue(value.hdrData.primaries[i].y);
                        p.primariesFloatEdits[i].second->setEnabled(bmd::HDRMode::Custom == value.hdrMode);
                    }

                    p.masteringLuminanceFloatEdits.first->setValue(value.hdrData.displayMasteringLuminance.min());
                    p.masteringLuminanceFloatEdits.first->setEnabled(bmd::HDRMode::Custom == value.hdrMode);
                    p.masteringLuminanceFloatEdits.second->setValue(value.hdrData.displayMasteringLuminance.max());
                    p.masteringLuminanceFloatEdits.second->setEnabled(bmd::HDRMode::Custom == value.hdrMode);

                    p.maxCLLSlider->setValue(value.hdrData.maxCLL);
                    p.maxCLLSlider->setEnabled(bmd::HDRMode::Custom == value.hdrMode);
                    p.maxFALLSlider->setValue(value.hdrData.maxFALL);
                    p.maxFALLSlider->setEnabled(bmd::HDRMode::Custom == value.hdrMode);
                });
#endif // TLRENDER_BMD
        }

        DevicesTool::DevicesTool() :
            _p(new Private)
        {}

        DevicesTool::~DevicesTool()
        {}

        std::shared_ptr<DevicesTool> DevicesTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DevicesTool>(new DevicesTool);
            out->_init(context, app, parent);
            return out;
        }
    }
}
