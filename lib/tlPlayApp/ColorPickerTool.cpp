// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ColorPickerTool.h>

#include <tlPlayApp/MainWindow.h>

#include <tlPlay/Viewport.h>

#include <tlUI/ColorSwatch.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct ColorPickerTool::Private
        {
            image::Color4f color;
            std::shared_ptr<ui::ColorSwatch> swatch;
            std::vector<std::shared_ptr<ui::Label> > labels;
            std::shared_ptr<ui::VerticalLayout> layout;
            std::shared_ptr<observer::ValueObserver<image::Color4f> > colorPickerObserver;
        };

        void ColorPickerTool::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::ColorPicker,
                "tl::play_app::ColorPickerTool",
                app,
                context,
                parent);
            TLRENDER_P();

            p.swatch = ui::ColorSwatch::create(context);
            p.swatch->setSizeRole(ui::SizeRole::SwatchLarge);

            p.labels.push_back(ui::Label::create(context));
            p.labels.push_back(ui::Label::create(context));
            p.labels.push_back(ui::Label::create(context));
            p.labels.push_back(ui::Label::create(context));

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.swatch->setParent(p.layout);
            auto gridLayout = ui::GridLayout::create(context, p.layout);
            gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Red:", context, gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.labels[0]->setParent(gridLayout);
            gridLayout->setGridPos(p.labels[0], 0, 1);
            label = ui::Label::create("Green:", context, gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.labels[1]->setParent(gridLayout);
            gridLayout->setGridPos(p.labels[1], 1, 1);
            label = ui::Label::create("Blue:", context, gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.labels[2]->setParent(gridLayout);
            gridLayout->setGridPos(p.labels[2], 2, 1);
            label = ui::Label::create("Alpha:", context, gridLayout);
            gridLayout->setGridPos(label, 3, 0);
            p.labels[3]->setParent(gridLayout);
            gridLayout->setGridPos(p.labels[3], 3, 1);
            _setWidget(p.layout);

            _widgetUpdate();

            p.colorPickerObserver = observer::ValueObserver<image::Color4f>::create(
                mainWindow->getViewport()->observeColorPicker(),
                [this](const image::Color4f& value)
                {
                    _p->color = value;
                    _widgetUpdate();
                });
        }

        ColorPickerTool::ColorPickerTool() :
            _p(new Private)
        {}

        ColorPickerTool::~ColorPickerTool()
        {}

        std::shared_ptr<ColorPickerTool> ColorPickerTool::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ColorPickerTool>(new ColorPickerTool);
            out->_init(mainWindow, app, context, parent);
            return out;
        }

        void ColorPickerTool::_widgetUpdate()
        {
            TLRENDER_P();
            p.swatch->setColor(p.color);
            p.labels[0]->setText(string::Format("{0}").arg(p.color.r));
            p.labels[1]->setText(string::Format("{0}").arg(p.color.g));
            p.labels[2]->setText(string::Format("{0}").arg(p.color.b));
            p.labels[3]->setText(string::Format("{0}").arg(p.color.a));
        }
    }
}
