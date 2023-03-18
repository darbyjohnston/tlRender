// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ButtonWidget.h"

#include <tlUI/RowLayout.h>
#include <tlUI/TextButton.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct ButtonWidget::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
                std::shared_ptr<observer::ValueObserver<bool> > buttonObserver0;
                std::shared_ptr<observer::ValueObserver<bool> > buttonObserver1;
                std::shared_ptr<observer::ValueObserver<bool> > buttonObserver2;
            };

            void ButtonWidget::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("ButtonWidget", context);
                TLRENDER_P();

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
                imaging::FontInfo fontInfo;
                fontInfo.size = 32;
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
                textButton0->setParent(p.layout);
                auto hLayout = ui::HorizontalLayout::create(context);
                hLayout->setParent(p.layout);
                textButton1->setParent(hLayout);
                textButton2->setParent(hLayout);
                p.layout->setParent(shared_from_this());
            }

            ButtonWidget::ButtonWidget() :
                _p(new Private)
            {}

            ButtonWidget::~ButtonWidget()
            {}

            std::shared_ptr<ButtonWidget> ButtonWidget::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<ButtonWidget>(new ButtonWidget);
                out->_init(context);
                return out;
            }

            void ButtonWidget::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
