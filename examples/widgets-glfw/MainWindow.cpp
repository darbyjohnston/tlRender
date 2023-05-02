// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "BasicWidgets.h"
#include "Charts.h"
#include "GridLayouts.h"
#include "NumericWidgets.h"
#include "RowLayouts.h"
#include "ScrollAreas.h"

#include <tlUI/ButtonGroup.h>
#include <tlUI/ListButton.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollArea.h>
#include <tlUI/StackLayout.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct MainWindow::Private
            {
                std::shared_ptr<ui::ButtonGroup> buttonGroup;
                std::shared_ptr<ui::RowLayout> layout;
                std::shared_ptr<ui::StackLayout> stackLayout;
            };

            void MainWindow::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("MainWindow", context);
                TLRENDER_P();

                setBackgroundRole(ui::ColorRole::Window);

                std::vector<std::shared_ptr<ui::IButton> > buttons;
                const std::vector<std::string> buttonText =
                {
                    "Basic Widgets",
                    "Numeric Widgets",
                    "Charts",
                    "Row Layouts",
                    "Grid Layouts",
                    "Scroll Areas"
                };
                p.buttonGroup = ui::ButtonGroup::create(
                    ui::ButtonGroupType::Click,
                    context);
                for (const auto& text : buttonText)
                {
                    auto button = ui::ListButton::create(context);
                    button->setText(text);
                    buttons.push_back(button);
                    p.buttonGroup->addButton(button);
                }
                p.buttonGroup->setClickedCallback(
                    [this](int value)
                    {
                        _p->stackLayout->setCurrentIndex(value);
                    });

                auto basicWidgets = BasicWidgets::create(context);
                auto numericWidgets = NumericWidgets::create(context);
                auto charts = Charts::create(context);
                auto rowLayouts = RowLayouts::create(context);
                auto gridLayouts = GridLayouts::create(context);
                auto scrollAreas = ScrollAreas::create(context);

                p.layout = ui::HorizontalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                p.layout->setSpacingRole(ui::SizeRole::SpacingLarge);
                auto scrollArea = ui::ScrollArea::create(
                    context,
                    ui::ScrollType::Vertical,
                    p.layout);
                auto buttonLayout = ui::VerticalLayout::create(context, scrollArea);
                buttonLayout->setSpacingRole(ui::SizeRole::None);
                for (auto button : buttons)
                {
                    button->setParent(buttonLayout);
                }
                p.stackLayout = ui::StackLayout::create(context, p.layout);
                p.stackLayout->setHStretch(ui::Stretch::Expanding);
                p.stackLayout->setVStretch(ui::Stretch::Expanding);
                basicWidgets->setParent(p.stackLayout);
                numericWidgets->setParent(p.stackLayout);
                charts->setParent(p.stackLayout);
                rowLayouts->setParent(p.stackLayout);
                gridLayouts->setParent(p.stackLayout);
                scrollAreas->setParent(p.stackLayout);

                p.stackLayout->setCurrentWidget(scrollAreas);
            }

            MainWindow::MainWindow() :
                _p(new Private)
            {}

            MainWindow::~MainWindow()
            {}

            std::shared_ptr<MainWindow> MainWindow::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<MainWindow>(new MainWindow);
                out->_init(context);
                return out;
            }

            void MainWindow::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
