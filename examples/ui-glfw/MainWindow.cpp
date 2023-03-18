// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "ButtonWidget.h"
#include "RowLayoutWidget.h"

#include <tlUI/RowLayout.h>
#include <tlUI/StackLayout.h>
#include <tlUI/TextButton.h>

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
                std::shared_ptr<observer::ValueObserver<bool> > rowLayoutObserver;
                std::shared_ptr<observer::ValueObserver<bool> > buttonsObserver;
            };

            void MainWindow::_init(
                const std::shared_ptr<system::Context>& context)
            {
                Window::_init(context);
                TLRENDER_P();

                auto rowLayoutButton = ui::TextButton::create(context);
                rowLayoutButton->setText("Row Layouts");
                p.rowLayoutObserver = observer::ValueObserver<bool>::create(
                    rowLayoutButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(0);
                    },
                    observer::CallbackAction::Suppress);

                auto buttonButton = ui::TextButton::create(context);
                buttonButton->setText("Buttons");
                p.buttonsObserver = observer::ValueObserver<bool>::create(
                    buttonButton->observeClick(),
                    [this](bool)
                    {
                        _p->stackLayout->setCurrentIndex(1);
                    },
                    observer::CallbackAction::Suppress);

                auto rowLayoutWidget = RowLayoutWidget::create(context);
                auto buttonWidget = ButtonWidget::create(context);

                p.layout = ui::HorizontalLayout::create(context);
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto buttonLayout = ui::VerticalLayout::create(context);
                buttonLayout->setParent(p.layout);
                rowLayoutButton->setParent(buttonLayout);
                buttonButton->setParent(buttonLayout);
                p.stackLayout = ui::StackLayout::create(context);
                p.stackLayout->setStretch(ui::Stretch::Expanding, ui::Orientation::Horizontal);
                p.stackLayout->setStretch(ui::Stretch::Expanding, ui::Orientation::Vertical);
                p.stackLayout->setParent(p.layout);
                rowLayoutWidget->setParent(p.stackLayout);
                buttonWidget->setParent(p.stackLayout);
                p.layout->setParent(shared_from_this());
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
