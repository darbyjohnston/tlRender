// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewToolPrivate.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ViewportModel.h>

#include <dtk/ui/Bellows.h>
#include <dtk/ui/ColorSwatch.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/GridLayout.h>
#include <dtk/ui/GroupBox.h>
#include <dtk/ui/IntEditSlider.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>

namespace tl
{
    namespace play_app
    {
        struct BackgroundWidget::Private
        {
            std::shared_ptr<dtk::ComboBox> typeComboBox;
            std::shared_ptr<dtk::ColorSwatch> color0Swatch;
            std::shared_ptr<dtk::ColorSwatch> color1Swatch;
            std::shared_ptr<dtk::IntEditSlider> checkersSizeSlider;
            std::shared_ptr<dtk::GridLayout> layout;

            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > optionsObservers;
        };

        void BackgroundWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::IWidget>& parent)
        {
            dtk::IWidget::_init(context, "tl::play_app::BackgroundWidget", parent);
            DTK_P();

            p.typeComboBox = dtk::ComboBox::create(
                context,
                timeline::getBackgroundLabels());
            p.typeComboBox->setHStretch(dtk::Stretch::Expanding);

            p.color0Swatch = dtk::ColorSwatch::create(context);
            p.color0Swatch->setEditable(true);
            p.color0Swatch->setHStretch(dtk::Stretch::Expanding);
            p.color1Swatch = dtk::ColorSwatch::create(context);
            p.color1Swatch->setEditable(true);
            p.color1Swatch->setHStretch(dtk::Stretch::Expanding);
            p.checkersSizeSlider = dtk::IntEditSlider::create(context);
            p.checkersSizeSlider->setRange(dtk::RangeI(10, 100));

            p.layout = dtk::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            auto label = dtk::Label::create(context, "Type:", p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.typeComboBox->setParent(p.layout);
            p.layout->setGridPos(p.typeComboBox, 0, 1);
            label = dtk::Label::create(context, "Color 0:", p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.color0Swatch->setParent(p.layout);
            p.layout->setGridPos(p.color0Swatch, 1, 1);
            label = dtk::Label::create(context, "Color 1:", p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.color1Swatch->setParent(p.layout);
            p.layout->setGridPos(p.color1Swatch, 2, 1);
            label = dtk::Label::create(context, "Checkers size:", p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.checkersSizeSlider->setParent(p.layout);
            p.layout->setGridPos(p.checkersSizeSlider, 3, 1);

            p.optionsObservers = dtk::ValueObserver<timeline::BackgroundOptions>::create(
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

            p.color0Swatch->setCallback(
                [appWeak](const dtk::Color4F& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.color0 = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.color1Swatch->setCallback(
                [appWeak](const dtk::Color4F& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.color1 = value;
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<BackgroundWidget>(new BackgroundWidget);
            out->_init(context, app, parent);
            return out;
        }

        void BackgroundWidget::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void BackgroundWidget::sizeHintEvent(const dtk::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void BackgroundWidget::_optionsUpdate(const timeline::BackgroundOptions& value)
        {
            DTK_P();
            p.typeComboBox->setCurrentIndex(static_cast<int>(value.type));
            p.color0Swatch->setColor(value.color0);
            p.color1Swatch->setColor(value.color1);
            p.checkersSizeSlider->setValue(value.checkersSize.w);
        }

        struct ViewTool::Private
        {
            std::shared_ptr<BackgroundWidget> backgroundWidget;
        };

        void ViewTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::View,
                "tl::play_app::ViewTool",
                parent);
            DTK_P();

            p.backgroundWidget = BackgroundWidget::create(context, app);

            auto layout = dtk::VerticalLayout::create(context);
            auto bellows = dtk::Bellows::create(context, "Background", layout);
            bellows->setWidget(p.backgroundWidget);
            auto scrollWidget = dtk::ScrollWidget::create(context);
            scrollWidget->setBorder(false);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);
        }

        ViewTool::ViewTool() :
            _p(new Private)
        {}

        ViewTool::~ViewTool()
        {}

        std::shared_ptr<ViewTool> ViewTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewTool>(new ViewTool);
            out->_init(context, app, parent);
            return out;
        }
    }
}
