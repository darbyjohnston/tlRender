// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include <tlUI/RowLayout.h>
#include <tlUI/Spacer.h>
#include <tlUI/TextButton.h>
#include <tlUI/TextLabel.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct MainWindow::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
                std::shared_ptr<observer::ValueObserver<bool> > buttonObserver0;
                std::shared_ptr<observer::ValueObserver<bool> > buttonObserver1;
                std::shared_ptr<observer::ValueObserver<bool> > buttonObserver2;
            };

            void MainWindow::_init(
                const std::shared_ptr<system::Context>& context)
            {
                Window::_init(context);
                TLRENDER_P();

                auto textLabel0 = ui::TextLabel::create(context);
                textLabel0->setText("Text Label 0");

                auto textLabel1 = ui::TextLabel::create(context);
                textLabel1->setText("Text Label 0");
                imaging::FontInfo fontInfo;
                fontInfo.size = 32;
                textLabel1->setFontInfo(fontInfo);

                auto textButton0 = ui::TextButton::create(context);
                textButton0->setText("Text Button 0");
                p.buttonObserver0 = observer::ValueObserver<bool>::create(
                    textButton0->observeClick(),
                    [this](bool)
                    {
                        std::cout << "Text Button 0" << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto textButton1 = ui::TextButton::create(context);
                textButton1->setText("Text Button 1");
                textButton1->setFontInfo(fontInfo);
                p.buttonObserver1 = observer::ValueObserver<bool>::create(
                    textButton1->observeClick(),
                    [this](bool)
                    {
                        std::cout << "Text Button 1" << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto textButton2 = ui::TextButton::create(context);
                textButton2->setText("Text Button 2");
                p.buttonObserver2 = observer::ValueObserver<bool>::create(
                    textButton2->observeClick(),
                    [this](bool)
                    {
                        std::cout << "Text Button 2" << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                p.layout = ui::VerticalLayout::create(context);
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto hLayout = ui::HorizontalLayout::create(context);
                hLayout->setParent(p.layout);
                textLabel0->setParent(hLayout);
                textLabel1->setParent(hLayout);
                textButton0->setParent(p.layout);
                hLayout = ui::HorizontalLayout::create(context);
                hLayout->setParent(p.layout);
                textButton1->setParent(hLayout);
                textButton2->setParent(hLayout);
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
                TLRENDER_P();
                p.layout->setGeometry(value);
            }
        }
    }
}
