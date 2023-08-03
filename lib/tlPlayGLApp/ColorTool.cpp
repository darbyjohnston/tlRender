// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ColorToolPrivate.h>

#include <tlPlayGLApp/App.h>

#include <tlPlay/ColorModel.h>

#include <tlUI/Bellows.h>
#include <tlUI/CheckBox.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

namespace tl
{
    namespace play_gl
    {
        struct LevelsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<ui::FloatEditSlider> > sliders;
            std::shared_ptr<ui::GridLayout> layout;

            std::map<std::string, std::shared_ptr<observer::ValueObserver<float> > > sliderObservers;
            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > optionsObservers;
        };

        void LevelsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init("tl::play_gl::LevelsWidget", context, parent);
            TLRENDER_P();

            p.enabledCheckBox = ui::CheckBox::create("Enabled", context);

            p.sliders["InLow"] = ui::FloatEditSlider::create(context);
            p.sliders["InLow"]->getModel()->setDefaultValue(0.F);
            p.sliders["InHigh"] = ui::FloatEditSlider::create(context);
            p.sliders["InHigh"]->getModel()->setDefaultValue(1.F);
            p.sliders["Gamma"] = ui::FloatEditSlider::create(context);
            p.sliders["Gamma"]->getModel()->setRange(math::FloatRange(.1F, 4.F));
            p.sliders["Gamma"]->getModel()->setDefaultValue(1.F);
            p.sliders["OutLow"] = ui::FloatEditSlider::create(context);
            p.sliders["OutLow"]->getModel()->setDefaultValue(0.F);
            p.sliders["OutHigh"] = ui::FloatEditSlider::create(context);
            p.sliders["OutHigh"]->getModel()->setDefaultValue(1.F);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = ui::Label::create("In low:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.sliders["InLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["InLow"], 1, 1);
            label = ui::Label::create("In high:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.sliders["InHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["InHigh"], 2, 1);
            label = ui::Label::create("Gamma:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.sliders["Gamma"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Gamma"], 3, 1);
            label = ui::Label::create("Out low:", context, p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sliders["OutLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["OutLow"], 4, 1);
            label = ui::Label::create("Out high:", context, p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.sliders["OutHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["OutHigh"], 5, 1);

            p.optionsObservers = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->getColorModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _p->enabledCheckBox->setChecked(value.levelsEnabled);
                    _p->sliders["InLow"]->getModel()->setValue(value.levels.inLow);
                    _p->sliders["InHigh"]->getModel()->setValue(value.levels.inHigh);
                    _p->sliders["Gamma"]->getModel()->setValue(value.levels.gamma);
                    _p->sliders["OutLow"]->getModel()->setValue(value.levels.outLow);
                    _p->sliders["OutHigh"]->getModel()->setValue(value.levels.outHigh);
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getDisplayOptions();
                        options.levelsEnabled = value;
                        app->getColorModel()->setDisplayOptions(options);
                    }
                });

            p.sliderObservers["InLow"] = observer::ValueObserver<float>::create(
                p.sliders["InLow"]->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getDisplayOptions();
                        options.levelsEnabled = true;
                        options.levels.inLow = value;
                        app->getColorModel()->setDisplayOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);
            p.sliderObservers["InHigh"] = observer::ValueObserver<float>::create(
                p.sliders["InHigh"]->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getDisplayOptions();
                        options.levelsEnabled = true;
                        options.levels.inHigh = value;
                        app->getColorModel()->setDisplayOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);
            p.sliderObservers["Gamma"] = observer::ValueObserver<float>::create(
                p.sliders["Gamma"]->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getDisplayOptions();
                        options.levelsEnabled = true;
                        options.levels.gamma = value;
                        app->getColorModel()->setDisplayOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);
            p.sliderObservers["OutLow"] = observer::ValueObserver<float>::create(
                p.sliders["OutLow"]->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getDisplayOptions();
                        options.levelsEnabled = true;
                        options.levels.outLow = value;
                        app->getColorModel()->setDisplayOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);
            p.sliderObservers["OutHigh"] = observer::ValueObserver<float>::create(
                p.sliders["OutHigh"]->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getDisplayOptions();
                        options.levelsEnabled = true;
                        options.levels.outHigh = value;
                        app->getColorModel()->setDisplayOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);
        }

        LevelsWidget::LevelsWidget() :
            _p(new Private)
        {}

        LevelsWidget::~LevelsWidget()
        {}

        std::shared_ptr<LevelsWidget> LevelsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<LevelsWidget>(new LevelsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void LevelsWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void LevelsWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct ColorTool::Private
        {
            std::shared_ptr<LevelsWidget> levelsWidget;
            std::map<std::string, std::shared_ptr<ui::Bellows> > bellows;
        };

        void ColorTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Color,
                "tl::play_gl::ColorTool",
                app,
                context,
                parent);
            TLRENDER_P();

            p.levelsWidget = LevelsWidget::create(app, context);

            auto layout = ui::VerticalLayout::create(context);
            layout->setSpacingRole(ui::SizeRole::None);
            p.bellows["Config"] = ui::Bellows::create("Configuration", context);
            p.bellows["Config"]->setParent(layout);
            p.bellows["LUT"] = ui::Bellows::create("LUT", context);
            p.bellows["LUT"]->setParent(layout);
            p.bellows["Color"] = ui::Bellows::create("Color Controls", context);
            p.bellows["Color"]->setParent(layout);
            p.bellows["Levels"] = ui::Bellows::create("Levels", context);
            p.bellows["Levels"]->setParent(layout);
            p.bellows["Levels"]->setWidget(p.levelsWidget);
            p.bellows["EXR"] = ui::Bellows::create("EXR Display", context);
            p.bellows["EXR"]->setParent(layout);
            p.bellows["SoftClip"] = ui::Bellows::create("Soft Clip", context);
            p.bellows["SoftClip"]->setParent(layout);
            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);
        }

        ColorTool::ColorTool() :
            _p(new Private)
        {}

        ColorTool::~ColorTool()
        {}

        std::shared_ptr<ColorTool> ColorTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<ColorTool>(new ColorTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
