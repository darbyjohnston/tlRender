// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "ButtonWidget.h"
#include "RowLayoutWidget.h"

#include <tlUI/ListButton.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollArea.h>
#include <tlUI/Spacer.h>
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
                std::shared_ptr<observer::ValueObserver<bool> > buttonsObserver;
                std::shared_ptr<observer::ValueObserver<bool> > rowLayoutObserver;
            };

            void MainWindow::_init(
                const std::shared_ptr<system::Context>& context)
            {
                Window::_init(context);
                TLRENDER_P();

                auto buttonButton = ui::ListButton::create(context);
                buttonButton->setText("Buttons");
                buttonButton->setBackgroundRole(ui::ColorRole::None);
                p.buttonsObserver = observer::ValueObserver<bool>::create(
                    buttonButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(0);
                    },
                    observer::CallbackAction::Suppress);

                auto rowLayoutButton = ui::ListButton::create(context);
                rowLayoutButton->setText("Row Layouts");
                rowLayoutButton->setBackgroundRole(ui::ColorRole::None);
                p.rowLayoutObserver = observer::ValueObserver<bool>::create(
                    rowLayoutButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(1);
                    },
                    observer::CallbackAction::Suppress);

                auto buttonWidget = ButtonWidget::create(context);
                auto rowLayoutWidget = RowLayoutWidget::create(context);

                p.layout = ui::HorizontalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto scrollArea = ui::ScrollArea::create(
                    context,
                    ui::ScrollAreaType::Vertical,
                    p.layout);
                auto spacer = ui::Spacer::create(context, p.layout);
                auto buttonLayout = ui::VerticalLayout::create(context, scrollArea);
                buttonLayout->setSpacingRole(ui::SizeRole::None);
                buttonButton->setParent(buttonLayout);
                rowLayoutButton->setParent(buttonLayout);
                p.stackLayout = ui::StackLayout::create(context, p.layout);
                p.stackLayout->setStretch(ui::Stretch::Expanding, ui::Orientation::Horizontal);
                p.stackLayout->setStretch(ui::Stretch::Expanding, ui::Orientation::Vertical);
                buttonWidget->setParent(p.stackLayout);
                rowLayoutWidget->setParent(p.stackLayout);
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
                Window::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
