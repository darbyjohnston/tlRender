// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ButtonWidget.h"

#include <tlUI/GroupBox.h>
#include <tlUI/PushButton.h>
#include <tlUI/ToolButton.h>
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

                auto pushButton0 = ui::PushButton::create(context);
                pushButton0->setText("Click");
                p.observers["pushButton0"] = observer::ValueObserver<bool>::create(
                    pushButton0->observeClick(),
                    [this](bool)
                    {
                        std::cout << "Click" << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto pushButton1 = ui::PushButton::create(context);
                pushButton1->setCheckable(true);
                pushButton1->setChecked(true);
                pushButton1->setText("Toggle");
                pushButton1->setIcon("Settings");
                p.observers["pushButton1"] = observer::ValueObserver<bool>::create(
                    pushButton1->observeChecked(),
                    [this](bool value)
                    {
                        std::cout << "Toggle: " << value << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto toolButton0 = ui::ToolButton::create(context);
                toolButton0->setCheckable(true);
                toolButton0->setChecked(true);
                toolButton0->setIcon("PlaybackStop");
                p.observers["toolButton0"] = observer::ValueObserver<bool>::create(
                    toolButton0->observeChecked(),
                    [this](bool value)
                    {
                        std::cout << "Stop: " << value << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                auto toolButton1 = ui::ToolButton::create(context);
                toolButton1->setCheckable(true);
                toolButton1->setText("Forward");
                toolButton1->setIcon("PlaybackForward");
                p.observers["toolButton1"] = observer::ValueObserver<bool>::create(
                    toolButton1->observeChecked(),
                    [this](bool value)
                    {
                        std::cout << "Forward" << value << std::endl;
                    },
                    observer::CallbackAction::Suppress);

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                auto groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Push Buttons");
                auto hLayout = ui::HorizontalLayout::create(context, groupBox);
                pushButton0->setParent(hLayout);
                pushButton1->setParent(hLayout);
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Tool Buttons");
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                toolButton0->setParent(hLayout);
                toolButton1->setParent(hLayout);
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
