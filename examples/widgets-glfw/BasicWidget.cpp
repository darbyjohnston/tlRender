// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "BasicWidget.h"

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
            struct BasicWidget::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
                std::map<std::string, std::shared_ptr<observer::ValueObserver<bool> > > observers;
            };

            void BasicWidget::_init(
                const std::shared_ptr<system::Context>& context)
            {
                IWidget::_init("BasicWidget", context);
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
                toolButton0->setIcon("PlaybackReverse");

                auto toolButton1 = ui::ToolButton::create(context);
                toolButton1->setCheckable(true);
                toolButton1->setIcon("PlaybackStop");

                auto toolButton2 = ui::ToolButton::create(context);
                toolButton2->setCheckable(true);
                toolButton2->setText("Forward");
                toolButton2->setIcon("PlaybackForward");

                p.layout = ui::VerticalLayout::create(context, shared_from_this());
                auto groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Push Buttons");
                auto hLayout = ui::HorizontalLayout::create(context, groupBox);
                pushButton0->setParent(hLayout);
                pushButton1->setParent(hLayout);
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Tool Buttons");
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
                toolButton0->setParent(hLayout);
                toolButton1->setParent(hLayout);
                toolButton2->setParent(hLayout);
            }

            BasicWidget::BasicWidget() :
                _p(new Private)
            {}

            BasicWidget::~BasicWidget()
            {}

            std::shared_ptr<BasicWidget> BasicWidget::create(
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<BasicWidget>(new BasicWidget);
                out->_init(context);
                return out;
            }

            void BasicWidget::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
