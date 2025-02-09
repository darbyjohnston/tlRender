// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/IToolWidget.h>

#include <tlPlayApp/App.h>

#include <dtk/ui/Icon.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/ToolButton.h>
#include <dtk/ui/RowLayout.h>

namespace tl
{
    namespace play_app
    {
        struct IToolWidget::Private
        {
            Tool tool;
            std::shared_ptr<dtk::Icon> icon;
            std::shared_ptr<dtk::Label> label;
            std::shared_ptr<dtk::ToolButton> closeButton;
            std::shared_ptr<dtk::VerticalLayout> toolLayout;
            std::shared_ptr<dtk::VerticalLayout> layout;
        };

        void IToolWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            Tool tool,
            const std::string& objectName,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, objectName, parent);
            DTK_P();

            _app = app;
            p.tool = tool;

            p.icon = dtk::Icon::create(context, getIcon(tool));
            p.icon->setMarginRole(dtk::SizeRole::MarginSmall);

            p.label = dtk::Label::create(context, getText(tool));
            p.label->setMarginRole(dtk::SizeRole::MarginSmall);
            p.label->setHStretch(dtk::Stretch::Expanding);

            p.closeButton = dtk::ToolButton::create(context);
            p.closeButton->setIcon("Close");

            p.layout = dtk::VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            auto hLayout = dtk::HorizontalLayout::create(context, p.layout);
            hLayout->setSpacingRole(dtk::SizeRole::None);
            //hLayout->setBackgroundRole(dtk::ColorRole::Button);
            p.icon->setParent(hLayout);
            p.label->setParent(hLayout);
            p.closeButton->setParent(hLayout);
            p.toolLayout = dtk::VerticalLayout::create(context, p.layout);
            p.toolLayout->setSpacingRole(dtk::SizeRole::None);
            p.toolLayout->setHStretch(dtk::Stretch::Expanding);
            p.toolLayout->setVStretch(dtk::Stretch::Expanding);

            auto appWeak = std::weak_ptr<App>(app);
            p.closeButton->setClickedCallback(
                [appWeak, tool]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getToolsModel()->setActiveTool(-1);
                    }
                });
        }

        IToolWidget::IToolWidget() :
            _p(new Private)
        {}

        IToolWidget::~IToolWidget()
        {}

        void IToolWidget::setGeometry(const dtk::Box2I & value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void IToolWidget::sizeHintEvent(const dtk::SizeHintEvent & event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void IToolWidget::_setWidget(const std::shared_ptr<dtk::IWidget>& value)
        {
            value->setHStretch(dtk::Stretch::Expanding);
            value->setVStretch(dtk::Stretch::Expanding);
            value->setParent(_p->toolLayout);
        }
    }
}
