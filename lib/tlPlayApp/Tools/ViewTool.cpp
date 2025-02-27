// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Tools/ViewToolPrivate.h>

#include <tlPlayApp/Models/ViewportModel.h>
#include <tlPlayApp/App.h>

#include <dtk/ui/Bellows.h>
#include <dtk/ui/ColorSwatch.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/FormLayout.h>
#include <dtk/ui/GroupBox.h>
#include <dtk/ui/IntEditSlider.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>

namespace tl
{
    namespace play
    {
        struct BackgroundWidget::Private
        {
            std::shared_ptr<dtk::ComboBox> typeComboBox;
            std::shared_ptr<dtk::ColorSwatch> solidSwatch;
            std::pair< std::shared_ptr<dtk::ColorSwatch>, std::shared_ptr<dtk::ColorSwatch> > checkersSwatch;
            std::shared_ptr<dtk::IntEditSlider> checkersSizeSlider;
            std::pair< std::shared_ptr<dtk::ColorSwatch>, std::shared_ptr<dtk::ColorSwatch> > gradientSwatch;
            std::shared_ptr<dtk::FormLayout> layout;

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

            p.solidSwatch = dtk::ColorSwatch::create(context);
            p.solidSwatch->setEditable(true);
            p.solidSwatch->setHAlign(dtk::HAlign::Left);

            p.checkersSwatch.first = dtk::ColorSwatch::create(context);
            p.checkersSwatch.first->setEditable(true);
            p.checkersSwatch.second = dtk::ColorSwatch::create(context);
            p.checkersSwatch.second->setEditable(true);
            p.checkersSizeSlider = dtk::IntEditSlider::create(context);
            p.checkersSizeSlider->setRange(dtk::RangeI(10, 100));

            p.gradientSwatch.first = dtk::ColorSwatch::create(context);
            p.gradientSwatch.first->setEditable(true);
            p.gradientSwatch.second = dtk::ColorSwatch::create(context);
            p.gradientSwatch.second->setEditable(true);

            p.layout = dtk::FormLayout::create(context, shared_from_this());
            p.layout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            p.layout->addRow("Type:", p.typeComboBox);
            p.layout->addRow("Color:", p.solidSwatch);
            p.layout->addRow("Color 1:", p.checkersSwatch.first);
            p.layout->addRow("Color 2:", p.checkersSwatch.second);
            p.layout->addRow("Size:", p.checkersSizeSlider);
            p.layout->addRow("Color 1:", p.gradientSwatch.first);
            p.layout->addRow("Color 2:", p.gradientSwatch.second);

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

            p.solidSwatch->setCallback(
                [appWeak](const dtk::Color4F& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.solidColor = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.checkersSwatch.first->setCallback(
                [appWeak](const dtk::Color4F& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.checkersColor.first = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.checkersSwatch.second->setCallback(
                [appWeak](const dtk::Color4F& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.checkersColor.second = value;
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

            p.gradientSwatch.first->setCallback(
                [appWeak](const dtk::Color4F& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.gradientColor.first = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.gradientSwatch.second->setCallback(
                [appWeak](const dtk::Color4F& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getViewportModel()->getBackgroundOptions();
                        options.gradientColor.second = value;
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
            p.solidSwatch->setColor(value.solidColor);
            p.checkersSwatch.first->setColor(value.checkersColor.first);
            p.checkersSwatch.second->setColor(value.checkersColor.second);
            p.checkersSizeSlider->setValue(value.checkersSize.w);
            p.gradientSwatch.first->setColor(value.gradientColor.first);
            p.gradientSwatch.second->setColor(value.gradientColor.second);

            p.layout->setRowVisible(p.solidSwatch, value.type == timeline::Background::Solid);
            p.layout->setRowVisible(p.checkersSwatch.first, value.type == timeline::Background::Checkers);
            p.layout->setRowVisible(p.checkersSwatch.second, value.type == timeline::Background::Checkers);
            p.layout->setRowVisible(p.checkersSizeSlider, value.type == timeline::Background::Checkers);
            p.layout->setRowVisible(p.gradientSwatch.first, value.type == timeline::Background::Gradient);
            p.layout->setRowVisible(p.gradientSwatch.second, value.type == timeline::Background::Gradient);
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
