// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ViewToolPrivate.h>

#include <tlPlayGLApp/App.h>

#include <tlPlay/ViewportModel.h>

#include <tlUI/Bellows.h>
#include <tlUI/ColorSwatch.h>
#include <tlUI/ComboBox.h>
#include <tlUI/GridLayout.h>
#include <tlUI/GroupBox.h>
#include <tlUI/IntEditSlider.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

namespace tl
{
    namespace play_gl
    {
        struct BackgroundWidget::Private
        {
            std::shared_ptr<ui::ComboBox> typeComboBox;
            std::shared_ptr<ui::ColorSwatch> solidColorSwatch;
            std::shared_ptr<ui::ColorSwatch> checkersColor0Swatch;
            std::shared_ptr<ui::ColorSwatch> checkersColor1Swatch;
            std::shared_ptr<ui::IntEditSlider> checkersSizeSlider;
            std::shared_ptr<ui::VerticalLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::BackgroundOptions> > optionsObservers;
        };

        void BackgroundWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init("tl::play_gl::BackgroundWidget", context, parent);
            TLRENDER_P();

            p.typeComboBox = ui::ComboBox::create(
                timeline::getBackgroundLabels(),
                context);

            p.solidColorSwatch = ui::ColorSwatch::create(context);
            p.solidColorSwatch->setEditable(true);

            p.checkersColor0Swatch = ui::ColorSwatch::create(context);
            p.checkersColor0Swatch->setEditable(true);
            p.checkersColor1Swatch = ui::ColorSwatch::create(context);
            p.checkersColor1Swatch->setEditable(true);
            p.checkersSizeSlider = ui::IntEditSlider::create(context);
            p.checkersSizeSlider->setRange(math::IntRange(10, 100));

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.typeComboBox->setParent(p.layout);
            auto groupBox = ui::GroupBox::create("Solid", context, p.layout);
            p.solidColorSwatch->setParent(groupBox);
            groupBox = ui::GroupBox::create("Checkers", context, p.layout);
            auto gridLayout = ui::GridLayout::create(context, groupBox);
            auto label = ui::Label::create("Color 0:", context, gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.checkersColor0Swatch->setParent(gridLayout);
            gridLayout->setGridPos(p.checkersColor0Swatch, 0, 1);
            label = ui::Label::create("Color 1:", context, gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.checkersColor1Swatch->setParent(gridLayout);
            gridLayout->setGridPos(p.checkersColor1Swatch, 1, 1);
            label = ui::Label::create("Size:", context, gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.checkersSizeSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.checkersSizeSlider, 2, 1);

            p.optionsObservers = observer::ValueObserver<timeline::BackgroundOptions>::create(
                app->getViewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _optionsUpdate(value);
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.typeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.type = static_cast<timeline::Background>(value);
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.solidColorSwatch->setCallback(
                [appWeak](const image::Color4f& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.solidColor = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.checkersColor0Swatch->setCallback(
                [appWeak](const image::Color4f& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.checkersColor0 = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.checkersColor1Swatch->setCallback(
                [appWeak](const image::Color4f& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.checkersColor1 = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.checkersSizeSlider->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.checkersSize.w = value;
                        options.checkersSize.h = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });
        }

        BackgroundWidget::BackgroundWidget() :
            _p(new Private)
        {}

        BackgroundWidget::~BackgroundWidget()
        {}

        std::shared_ptr<BackgroundWidget> BackgroundWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<BackgroundWidget>(new BackgroundWidget);
            out->_init(app, context, parent);
            return out;
        }

        void BackgroundWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void BackgroundWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        void BackgroundWidget::_optionsUpdate(const timeline::BackgroundOptions& value)
        {
            TLRENDER_P();
            p.typeComboBox->setCurrentIndex(static_cast<int>(value.type));
            p.solidColorSwatch->setColor(value.solidColor);
            p.checkersColor0Swatch->setColor(value.checkersColor0);
            p.checkersColor1Swatch->setColor(value.checkersColor1);
            p.checkersSizeSlider->setValue(value.checkersSize.w);
        }

        struct ViewTool::Private
        {
            std::shared_ptr<BackgroundWidget> backgroundWidget;
        };

        void ViewTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::View,
                "tl::play_gl::ViewTool",
                app,
                context,
                parent);
            TLRENDER_P();

            p.backgroundWidget = BackgroundWidget::create(app, context);

            auto layout = ui::VerticalLayout::create(context);
            auto bellows = ui::Bellows::create("Background", context, layout);
            bellows->setWidget(p.backgroundWidget);
            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);
        }

        ViewTool::ViewTool() :
            _p(new Private)
        {}

        ViewTool::~ViewTool()
        {}

        std::shared_ptr<ViewTool> ViewTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewTool>(new ViewTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
