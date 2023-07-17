// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/IToolWidget.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/Label.h>
#include <tlUI/ToolButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace play_gl
    {
        struct IToolWidget::Private
        {
            Tool tool;
            std::shared_ptr<ui::Label> label;
            std::shared_ptr<ui::ToolButton> closeButton;
            std::shared_ptr<ui::VerticalLayout> layout;
        };

        void IToolWidget::_init(
            Tool tool,
            const std::string& name,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            TLRENDER_P();

            _app = app;
            p.tool = tool;

            p.label = ui::Label::create(context);
            p.label->setText(getText(tool));
            p.label->setMarginRole(ui::SizeRole::MarginSmall);
            p.label->setHStretch(ui::Stretch::Expanding);

            p.closeButton = ui::ToolButton::create(context);
            p.closeButton->setIcon("Close");

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            p.label->setParent(hLayout);
            p.closeButton->setParent(hLayout);

            auto appWeak = std::weak_ptr<App>(app);
            p.closeButton->setClickedCallback(
                [appWeak, tool]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getToolsModel()->setToolVisible(tool, false);
                    }
                });
        }

        IToolWidget::IToolWidget() :
            _p(new Private)
        {}

        IToolWidget::~IToolWidget()
        {}

        void IToolWidget::setGeometry(const math::BBox2i & value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void IToolWidget::sizeHintEvent(const ui::SizeHintEvent & event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
