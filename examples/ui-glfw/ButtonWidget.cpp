// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ButtonWidget.h"

#include <tlUI/GroupBox.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            struct ButtonWidget::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
                std::map<std::string, std::shared_ptr<observer::ValueObserver<bool> > > observers;
            };

            void ButtonWidget::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("ButtonWidget", context);
                TLRENDER_P();

                auto button0 = ui::PushButton::create(context);
                button0->setText("Button 0");
                p.observers["TextButton0"] = observer::ValueObserver<bool>::create(
                    button0->observeClick(),
                    [this](bool)
                    {
                        std::cout << "Button 0" << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto button1 = ui::PushButton::create(context);
                button1->setText("Button 1");
                p.observers["TextButton1"] = observer::ValueObserver<bool>::create(
                    button1->observeClick(),
                    [this](bool)
                    {
                        std::cout << "Button 1" << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto button2 = ui::PushButton::create(context);
                button2->setCheckable(true);
                button2->setText("Button 2");
                button2->setIcon("PlaybackForward");
                p.observers["TextButton2"] = observer::ValueObserver<bool>::create(
                    button2->observeChecked(),
                    [this](bool value)
                    {
                        std::cout << "Button 2: " << value << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto button3 = ui::PushButton::create(context);
                button3->setCheckable(true);
                button3->setChecked(true);
                button3->setText("Button 3");
                button3->setIcon("PlaybackStop");
                p.observers["TextButton3"] = observer::ValueObserver<bool>::create(
                    button3->observeChecked(),
                    [this](bool value)
                    {
                        std::cout << "Button 3: " << value << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                auto groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Push Buttons");
                auto hLayout = ui::HorizontalLayout::create(context, groupBox);
                button0->setParent(hLayout);
                button1->setParent(hLayout);
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Checkable Buttons");
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                button2->setParent(hLayout);
                button3->setParent(hLayout);
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
