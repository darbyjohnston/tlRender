// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "BasicWidget.h"
#include "GridLayoutWidget.h"
#include "NumericWidget.h"
#include "RowLayoutWidget.h"

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
                std::shared_ptr<ui::RowLayout> layout;
                std::shared_ptr<ui::StackLayout> stackLayout;
                std::shared_ptr<observer::ValueObserver<bool> > basicObserver;
                std::shared_ptr<observer::ValueObserver<bool> > numericObserver;
                std::shared_ptr<observer::ValueObserver<bool> > rowLayoutObserver;
                std::shared_ptr<observer::ValueObserver<bool> > gridLayoutObserver;
            };

            void MainWindow::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("MainWindow", context);
                TLRENDER_P();

                setBackgroundRole(ui::ColorRole::Window);

                auto basicButton = ui::ListButton::create(context);
                basicButton->setText("Basic Widgets");
                p.basicObserver = observer::ValueObserver<bool>::create(
                    basicButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(0);
                    },
                    observer::CallbackAction::Suppress);

                auto numericButton = ui::ListButton::create(context);
                numericButton->setText("Numeric Widgets");
                p.numericObserver = observer::ValueObserver<bool>::create(
                    numericButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(1);
                    },
                    observer::CallbackAction::Suppress);

                auto rowLayoutButton = ui::ListButton::create(context);
                rowLayoutButton->setText("Row Layouts");
                p.rowLayoutObserver = observer::ValueObserver<bool>::create(
                    rowLayoutButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(2);
                    },
                    observer::CallbackAction::Suppress);

                auto gridLayoutButton = ui::ListButton::create(context);
                gridLayoutButton->setText("Grid Layouts");
                p.gridLayoutObserver = observer::ValueObserver<bool>::create(
                    gridLayoutButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(3);
                    },
                    observer::CallbackAction::Suppress);

                auto basicWidget = BasicWidget::create(context);
                auto numericWidget = NumericWidget::create(context);
                auto rowLayoutWidget = RowLayoutWidget::create(context);
                auto gridLayoutWidget = GridLayoutWidget::create(context);

                p.layout = ui::HorizontalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                p.layout->setSpacingRole(ui::SizeRole::SpacingLarge);
                auto scrollArea = ui::ScrollArea::create(
                    context,
                    ui::ScrollAreaType::Vertical,
                    p.layout);
                auto buttonLayout = ui::VerticalLayout::create(context, scrollArea);
                buttonLayout->setSpacingRole(ui::SizeRole::None);
                basicButton->setParent(buttonLayout);
                numericButton->setParent(buttonLayout);
                rowLayoutButton->setParent(buttonLayout);
                gridLayoutButton->setParent(buttonLayout);
                p.stackLayout = ui::StackLayout::create(context, p.layout);
                p.stackLayout->setStretch(ui::Stretch::Expanding, ui::Orientation::Horizontal);
                p.stackLayout->setStretch(ui::Stretch::Expanding, ui::Orientation::Vertical);
                basicWidget->setParent(p.stackLayout);
                numericWidget->setParent(p.stackLayout);
                rowLayoutWidget->setParent(p.stackLayout);
                gridLayoutWidget->setParent(p.stackLayout);

                p.stackLayout->setCurrentIndex(1);
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
