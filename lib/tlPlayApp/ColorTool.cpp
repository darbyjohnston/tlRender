// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ColorToolPrivate.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/ViewportModel.h>

#include <dtk/ui/Bellows.h>
#include <dtk/ui/ButtonGroup.h>
#include <dtk/ui/CheckBox.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/FileEdit.h>
#include <dtk/ui/FloatEditSlider.h>
#include <dtk/ui/GridLayout.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>
#include <dtk/ui/StackLayout.h>

namespace tl
{
    namespace play_app
    {
        struct OCIOWidget::Private
        {
            std::shared_ptr<play::OCIOModel> ocioModel;

            std::shared_ptr<dtk::CheckBox> enabledCheckBox;
            std::shared_ptr<dtk::FileEdit> fileEdit;
            std::shared_ptr<dtk::ComboBox> inputComboBox;
            std::shared_ptr<dtk::ComboBox> displayComboBox;
            std::shared_ptr<dtk::ComboBox> viewComboBox;
            std::shared_ptr<dtk::ComboBox> lookComboBox;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > optionsObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::OCIOOptions> > optionsObserver2;
            std::shared_ptr<dtk::ValueObserver<play::OCIOModelData> > dataObserver;
        };

        void OCIOWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            dtk::IWidget::_init(context, "tl::play_app::OCIOWidget", parent);
            DTK_P();
            
#if !defined(TLRENDER_OCIO)
            setEnabled(false);
#endif // TLRENDER_OCIO

            p.ocioModel = play::OCIOModel::create(context);

            p.enabledCheckBox = dtk::CheckBox::create(context, "Enabled");

            p.fileEdit = dtk::FileEdit::create(context);

            p.inputComboBox = dtk::ComboBox::create(context);
            p.inputComboBox->setHStretch(dtk::Stretch::Expanding);

            p.displayComboBox = dtk::ComboBox::create(context);
            p.displayComboBox->setHStretch(dtk::Stretch::Expanding);

            p.viewComboBox = dtk::ComboBox::create(context);
            p.viewComboBox->setHStretch(dtk::Stretch::Expanding);

            p.lookComboBox = dtk::ComboBox::create(context);
            p.lookComboBox->setHStretch(dtk::Stretch::Expanding);

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto gridLayout = dtk::GridLayout::create(context, p.layout);
            gridLayout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = dtk::Label::create(context, "File name:", gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.fileEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.fileEdit, 1, 1);
            label = dtk::Label::create(context, "Input:", gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.inputComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.inputComboBox, 2, 1);
            label = dtk::Label::create(context, "Display:", gridLayout);
            gridLayout->setGridPos(label, 3, 0);
            p.displayComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.displayComboBox, 3, 1);
            label = dtk::Label::create(context, "View:", gridLayout);
            gridLayout->setGridPos(label, 4, 0);
            p.viewComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.viewComboBox, 4, 1);
            label = dtk::Label::create(context, "Look:", gridLayout);
            gridLayout->setGridPos(label, 5, 0);
            p.lookComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.lookComboBox, 5, 1);

            p.optionsObserver = dtk::ValueObserver<timeline::OCIOOptions>::create(
                app->getColorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                    _p->enabledCheckBox->setChecked(value.enabled);
                    _p->fileEdit->setPath(std::filesystem::u8path(value.fileName));
                    _p->ocioModel->setOptions(value);
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.optionsObserver2 = dtk::ValueObserver<timeline::OCIOOptions>::create(
                p.ocioModel->observeOptions(),
                [appWeak](const timeline::OCIOOptions& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getColorModel()->setOCIOOptions(value);
                    }
                });

            p.dataObserver = dtk::ValueObserver<play::OCIOModelData>::create(
                p.ocioModel->observeData(),
                [this](const play::OCIOModelData& value)
                {
                    _p->enabledCheckBox->setChecked(value.enabled);
                    _p->fileEdit->setPath(std::filesystem::u8path(value.fileName));
                    _p->inputComboBox->setItems(value.inputs);
                    _p->inputComboBox->setCurrentIndex(value.inputIndex);
                    _p->displayComboBox->setItems(value.displays);
                    _p->displayComboBox->setCurrentIndex(value.displayIndex);
                    _p->viewComboBox->setItems(value.views);
                    _p->viewComboBox->setCurrentIndex(value.viewIndex);
                    _p->lookComboBox->setItems(value.looks);
                    _p->lookComboBox->setCurrentIndex(value.lookIndex);
                });

            p.enabledCheckBox->setCheckedCallback(
                [this](bool value)
                {
                    _p->ocioModel->setEnabled(value);
                });

            p.fileEdit->setCallback(
                [this](const std::filesystem::path& value)
                {
                    _p->ocioModel->setConfig(value.u8string());
                });

            p.inputComboBox->setIndexCallback(
                [this](int index)
                {
                    _p->ocioModel->setInputIndex(index);
                });
            p.displayComboBox->setIndexCallback(
                [this](int index)
                {
                    _p->ocioModel->setDisplayIndex(index);
                });
            p.viewComboBox->setIndexCallback(
                [this](int index)
                {
                    _p->ocioModel->setViewIndex(index);
                });
            p.lookComboBox->setIndexCallback(
                [this](int index)
                {
                    _p->ocioModel->setLookIndex(index);
                });
        }

        OCIOWidget::OCIOWidget() :
            _p(new Private)
        {}

        OCIOWidget::~OCIOWidget()
        {}

        std::shared_ptr<OCIOWidget> OCIOWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<OCIOWidget>(new OCIOWidget);
            out->_init(context, app, parent);
            return out;
        }

        void OCIOWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void OCIOWidget::sizeHintEvent(const dtk::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct LUTWidget::Private
        {
            std::shared_ptr<dtk::CheckBox> enabledCheckBox;
            std::shared_ptr<dtk::FileEdit> fileEdit;
            std::shared_ptr<dtk::ComboBox> orderComboBox;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::LUTOptions> > optionsObservers;
        };

        void LUTWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            dtk::IWidget::_init(context, "tl::play_app::LUTWidget", parent);
            DTK_P();
            
#if !defined(TLRENDER_OCIO)
            setEnabled(false);
#endif // TLRENDER_OCIO

            p.enabledCheckBox = dtk::CheckBox::create(context, "Enabled");

            p.fileEdit = dtk::FileEdit::create(context);

            p.orderComboBox = dtk::ComboBox::create(context, timeline::getLUTOrderLabels());
            p.orderComboBox->setHStretch(dtk::Stretch::Expanding);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = dtk::Label::create(context, "File name:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.fileEdit->setParent(p.layout);
            p.layout->setGridPos(p.fileEdit, 1, 1);
            label = dtk::Label::create(context, "Order:", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.orderComboBox->setParent(p.layout);
            p.layout->setGridPos(p.orderComboBox, 2, 1);

            p.optionsObservers = dtk::ValueObserver<timeline::LUTOptions>::create(
                app->getColorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _p->enabledCheckBox->setChecked(value.enabled);
                    _p->fileEdit->setPath(std::filesystem::u8path(value.fileName));
                    _p->orderComboBox->setCurrentIndex(static_cast<size_t>(value.order));
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getLUTOptions();
                        options.enabled = value;
                        app->getColorModel()->setLUTOptions(options);
                    }
                });

            p.fileEdit->setCallback(
                [appWeak](const std::filesystem::path& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getLUTOptions();
                        options.enabled = true;
                        options.fileName = value.u8string();
                        app->getColorModel()->setLUTOptions(options);
                    }
                });

            p.orderComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getLUTOptions();
                        options.enabled = true;
                        options.order = static_cast<timeline::LUTOrder>(value);
                        app->getColorModel()->setLUTOptions(options);
                    }
                });
        }

        LUTWidget::LUTWidget() :
            _p(new Private)
        {}

        LUTWidget::~LUTWidget()
        {}

        std::shared_ptr<LUTWidget> LUTWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<LUTWidget>(new LUTWidget);
            out->_init(context, app, parent);
            return out;
        }

        void LUTWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void LUTWidget::sizeHintEvent(const dtk::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct ColorWidget::Private
        {
            std::shared_ptr<dtk::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<dtk::FloatEditSlider> > sliders;
            std::shared_ptr<dtk::CheckBox> invertCheckBox;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > optionsObservers;
        };

        void ColorWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            dtk::IWidget::_init(context, "tl::play_app::ColorWidget", parent);
            DTK_P();

            p.enabledCheckBox = dtk::CheckBox::create(context, "Enabled");

            p.sliders["Add"] = dtk::FloatEditSlider::create(context);
            p.sliders["Add"]->setRange(dtk::RangeF(-1.F, 1.F));
            p.sliders["Add"]->setDefaultValue(0.F);
            p.sliders["Brightness"] = dtk::FloatEditSlider::create(context);
            p.sliders["Brightness"]->setRange(dtk::RangeF(0.F, 4.F));
            p.sliders["Brightness"]->setDefaultValue(1.F);
            p.sliders["Contrast"] = dtk::FloatEditSlider::create(context);
            p.sliders["Contrast"]->setRange(dtk::RangeF(0.F, 4.F));
            p.sliders["Contrast"]->setDefaultValue(1.F);
            p.sliders["Saturation"] = dtk::FloatEditSlider::create(context);
            p.sliders["Saturation"]->setRange(dtk::RangeF(0.F, 4.F));
            p.sliders["Saturation"]->setDefaultValue(1.F);
            p.sliders["Tint"] = dtk::FloatEditSlider::create(context);
            p.sliders["Tint"]->setDefaultValue(1.F);

            p.invertCheckBox = dtk::CheckBox::create(context, "Invert");

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = dtk::Label::create(context, "Add:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.sliders["Add"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Add"], 1, 1);
            label = dtk::Label::create(context, "Brightness:", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.sliders["Brightness"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Brightness"], 2, 1);
            label = dtk::Label::create(context, "Contrast:", p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.sliders["Contrast"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Contrast"], 3, 1);
            label = dtk::Label::create(context, "Saturation:", p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sliders["Saturation"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Saturation"], 4, 1);
            label = dtk::Label::create(context, "Tint:", p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.sliders["Tint"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Tint"], 5, 1);
            p.invertCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.invertCheckBox, 6, 0);

            p.optionsObservers = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->enabledCheckBox->setChecked(value.color.enabled);
                    _p->sliders["Add"]->setValue(value.color.add.x);
                    _p->sliders["Brightness"]->setValue(value.color.brightness.x);
                    _p->sliders["Contrast"]->setValue(value.color.contrast.x);
                    _p->sliders["Saturation"]->setValue(value.color.saturation.x);
                    _p->sliders["Tint"]->setValue(value.color.tint);
                    _p->invertCheckBox->setChecked(value.color.invert);
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Add"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.add.x = value;
                        options.color.add.y = value;
                        options.color.add.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Brightness"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.brightness.x = value;
                        options.color.brightness.y = value;
                        options.color.brightness.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Contrast"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.contrast.x = value;
                        options.color.contrast.y = value;
                        options.color.contrast.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Saturation"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.saturation.x = value;
                        options.color.saturation.y = value;
                        options.color.saturation.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Tint"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.tint = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.invertCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.invert = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        ColorWidget::ColorWidget() :
            _p(new Private)
        {}

        ColorWidget::~ColorWidget()
        {}

        std::shared_ptr<ColorWidget> ColorWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ColorWidget>(new ColorWidget);
            out->_init(context, app, parent);
            return out;
        }

        void ColorWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ColorWidget::sizeHintEvent(const dtk::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct LevelsWidget::Private
        {
            std::shared_ptr<dtk::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<dtk::FloatEditSlider> > sliders;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > optionsObservers;
        };

        void LevelsWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            dtk::IWidget::_init(context, "tl::play_app::LevelsWidget", parent);
            DTK_P();

            p.enabledCheckBox = dtk::CheckBox::create(context, "Enabled");

            p.sliders["InLow"] = dtk::FloatEditSlider::create(context);
            p.sliders["InLow"]->setDefaultValue(0.F);
            p.sliders["InHigh"] = dtk::FloatEditSlider::create(context);
            p.sliders["InHigh"]->setDefaultValue(1.F);
            p.sliders["Gamma"] = dtk::FloatEditSlider::create(context);
            p.sliders["Gamma"]->setRange(dtk::RangeF(.1F, 4.F));
            p.sliders["Gamma"]->setDefaultValue(1.F);
            p.sliders["OutLow"] = dtk::FloatEditSlider::create(context);
            p.sliders["OutLow"]->setDefaultValue(0.F);
            p.sliders["OutHigh"] = dtk::FloatEditSlider::create(context);
            p.sliders["OutHigh"]->setDefaultValue(1.F);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = dtk::Label::create(context, "In low:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.sliders["InLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["InLow"], 1, 1);
            label = dtk::Label::create(context, "In high:", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.sliders["InHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["InHigh"], 2, 1);
            label = dtk::Label::create(context, "Gamma:", p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.sliders["Gamma"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Gamma"], 3, 1);
            label = dtk::Label::create(context, "Out low:", p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sliders["OutLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["OutLow"], 4, 1);
            label = dtk::Label::create(context, "Out high:", p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.sliders["OutHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["OutHigh"], 5, 1);

            p.optionsObservers = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->enabledCheckBox->setChecked(value.levels.enabled);
                    _p->sliders["InLow"]->setValue(value.levels.inLow);
                    _p->sliders["InHigh"]->setValue(value.levels.inHigh);
                    _p->sliders["Gamma"]->setValue(value.levels.gamma);
                    _p->sliders["OutLow"]->setValue(value.levels.outLow);
                    _p->sliders["OutHigh"]->setValue(value.levels.outHigh);
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["InLow"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.inLow = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["InHigh"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.inHigh = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Gamma"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.gamma = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["OutLow"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.outLow = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["OutHigh"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.outHigh = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        LevelsWidget::LevelsWidget() :
            _p(new Private)
        {}

        LevelsWidget::~LevelsWidget()
        {}

        std::shared_ptr<LevelsWidget> LevelsWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<LevelsWidget>(new LevelsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void LevelsWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void LevelsWidget::sizeHintEvent(const dtk::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct EXRDisplayWidget::Private
        {
            std::shared_ptr<dtk::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<dtk::FloatEditSlider> > sliders;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > optionsObservers;
        };

        void EXRDisplayWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            dtk::IWidget::_init(context, "tl::play_app::EXRDisplayWidget", parent);
            DTK_P();

            p.enabledCheckBox = dtk::CheckBox::create(context, "Enabled");

            p.sliders["Exposure"] = dtk::FloatEditSlider::create(context);
            p.sliders["Exposure"]->setRange(dtk::RangeF(-10.F, 10.F));
            p.sliders["Exposure"]->setDefaultValue(0.F);
            p.sliders["Defog"] = dtk::FloatEditSlider::create(context);
            p.sliders["Defog"]->setDefaultValue(0.F);
            p.sliders["KneeLow"] = dtk::FloatEditSlider::create(context);
            p.sliders["KneeLow"]->setRange(dtk::RangeF(-3.F, 3.F));
            p.sliders["KneeLow"]->setDefaultValue(0.F);
            p.sliders["KneeHigh"] = dtk::FloatEditSlider::create(context);
            p.sliders["KneeHigh"]->setRange(dtk::RangeF(3.5F, 7.5F));
            p.sliders["KneeHigh"]->setDefaultValue(5.F);

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = dtk::Label::create(context, "Exposure:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.sliders["Exposure"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Exposure"], 1, 1);
            label = dtk::Label::create(context, "Defog:", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.sliders["Defog"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Defog"], 2, 1);
            label = dtk::Label::create(context, "Knee low:", p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.sliders["KneeLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["KneeLow"], 3, 1);
            label = dtk::Label::create(context, "Knee high:", p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sliders["KneeHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["KneeHigh"], 4, 1);

            p.optionsObservers = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->enabledCheckBox->setChecked(value.exrDisplay.enabled);
                    _p->sliders["Exposure"]->setValue(value.exrDisplay.exposure);
                    _p->sliders["Defog"]->setValue(value.exrDisplay.defog);
                    _p->sliders["KneeLow"]->setValue(value.exrDisplay.kneeLow);
                    _p->sliders["KneeHigh"]->setValue(value.exrDisplay.kneeHigh);
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Exposure"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.exposure = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Defog"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.defog = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["KneeLow"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.kneeLow = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["KneeHigh"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.kneeHigh = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        EXRDisplayWidget::EXRDisplayWidget() :
            _p(new Private)
        {}

        EXRDisplayWidget::~EXRDisplayWidget()
        {}

        std::shared_ptr<EXRDisplayWidget> EXRDisplayWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<EXRDisplayWidget>(new EXRDisplayWidget);
            out->_init(context, app, parent);
            return out;
        }

        void EXRDisplayWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void EXRDisplayWidget::sizeHintEvent(const dtk::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct SoftClipWidget::Private
        {
            std::shared_ptr<dtk::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<dtk::FloatEditSlider> > sliders;
            std::shared_ptr<dtk::VerticalLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > optionsObservers;
        };

        void SoftClipWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            dtk::IWidget::_init(context, "tl::play_app::SoftClipWidget", parent);
            DTK_P();

            p.enabledCheckBox = dtk::CheckBox::create(context, "Enabled");

            p.sliders["SoftClip"] = dtk::FloatEditSlider::create(context);
            p.sliders["SoftClip"]->setDefaultValue(0.F);

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.sliders["SoftClip"]->setParent(p.layout);

            p.optionsObservers = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->enabledCheckBox->setChecked(value.softClip.enabled);
                    _p->sliders["SoftClip"]->setValue(value.softClip.value);
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.softClip.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["SoftClip"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getDisplayOptions();
                        options.softClip.enabled = true;
                        options.softClip.value = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        SoftClipWidget::SoftClipWidget() :
            _p(new Private)
        {}

        SoftClipWidget::~SoftClipWidget()
        {}

        std::shared_ptr<SoftClipWidget> SoftClipWidget::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SoftClipWidget>(new SoftClipWidget);
            out->_init(context, app, parent);
            return out;
        }

        void SoftClipWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void SoftClipWidget::sizeHintEvent(const dtk::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _setSizeHint(_p->layout->getSizeHint());
        }

        struct ColorTool::Private
        {
            std::shared_ptr<OCIOWidget> ocioWidget;
            std::shared_ptr<LUTWidget> lutWidget;
            std::shared_ptr<ColorWidget> colorWidget;
            std::shared_ptr<LevelsWidget> levelsWidget;
            std::shared_ptr<EXRDisplayWidget> exrDisplayWidget;
            std::shared_ptr<SoftClipWidget> softClipWidget;
            std::map<std::string, std::shared_ptr<dtk::Bellows> > bellows;
        };

        void ColorTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Color,
                "tl::play_app::ColorTool",
                parent);
            DTK_P();

            p.ocioWidget = OCIOWidget::create(context, app);
            p.lutWidget = LUTWidget::create(context, app);
            p.colorWidget = ColorWidget::create(context, app);
            p.levelsWidget = LevelsWidget::create(context, app);
            p.exrDisplayWidget = EXRDisplayWidget::create(context, app);
            p.softClipWidget = SoftClipWidget::create(context, app);

            auto layout = dtk::VerticalLayout::create(context);
            layout->setSpacingRole(dtk::SizeRole::None);
            p.bellows["OCIO"] = dtk::Bellows::create(context, "OCIO");
            p.bellows["OCIO"]->setParent(layout);
            p.bellows["OCIO"]->setWidget(p.ocioWidget);
            p.bellows["LUT"] = dtk::Bellows::create(context, "LUT");
            p.bellows["LUT"]->setParent(layout);
            p.bellows["LUT"]->setWidget(p.lutWidget);
            p.bellows["Color"] = dtk::Bellows::create(context, "Color");
            p.bellows["Color"]->setParent(layout);
            p.bellows["Color"]->setWidget(p.colorWidget);
            p.bellows["Levels"] = dtk::Bellows::create(context, "Levels");
            p.bellows["Levels"]->setParent(layout);
            p.bellows["Levels"]->setWidget(p.levelsWidget);
            p.bellows["EXRDisplay"] = dtk::Bellows::create(context, "EXR Display");
            p.bellows["EXRDisplay"]->setParent(layout);
            p.bellows["EXRDisplay"]->setWidget(p.exrDisplayWidget);
            p.bellows["SoftClip"] = dtk::Bellows::create(context, "Soft Clip");
            p.bellows["SoftClip"]->setParent(layout);
            p.bellows["SoftClip"]->setWidget(p.softClipWidget);
            auto scrollWidget = dtk::ScrollWidget::create(context);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);
        }

        ColorTool::ColorTool() :
            _p(new Private)
        {}

        ColorTool::~ColorTool()
        {}

        std::shared_ptr<ColorTool> ColorTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ColorTool>(new ColorTool);
            out->_init(context, app, parent);
            return out;
        }
    }
}
