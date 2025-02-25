// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ToolsToolBar.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/Tools.h>

#include <dtk/ui/ButtonGroup.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct ToolsToolBar::Private
        {
            std::vector<Tool> tools;
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::shared_ptr<dtk::ButtonGroup> buttonGroup;
            std::vector<std::shared_ptr<dtk::ToolButton> > buttons;
            std::shared_ptr<dtk::HorizontalLayout> layout;
            std::shared_ptr<dtk::ValueObserver<Tool> > activeObserver;
        };

        void ToolsToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::ToolsToolBar",
                parent);
            DTK_P();

            p.tools = getToolsInToolbar();

            p.actions = actions;

            p.buttonGroup = dtk::ButtonGroup::create(context, dtk::ButtonGroupType::Toggle);
            for (const auto tool : p.tools)
            {
                auto button = dtk::ToolButton::create(context);
                auto action = p.actions[getLabel(tool)];
                button->setIcon(action->icon);
                button->setCheckable(action->checkable);
                button->setTooltip(action->toolTip);
                p.buttonGroup->addButton(button);
                p.buttons.push_back(button);
            }

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            for (const auto& button : p.buttons)
            {
                button->setParent(p.layout);
            }

            auto appWeak = std::weak_ptr<App>(app);
            p.buttonGroup->setCheckedCallback(
                [this, appWeak](int index, bool value)
                {
                    DTK_P();
                    if (index >= 0 && index < p.tools.size())
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getToolsModel()->setActiveTool(
                                value ? _p->tools[index] : Tool::None);
                        }
                    }
                });

            p.activeObserver = dtk::ValueObserver<Tool>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](Tool value)
                {
                    DTK_P();
                    for (size_t i = 0; i < p.tools.size() && i < p.buttons.size(); ++i)
                    {
                        p.buttons[i]->setChecked(value == p.tools[i]);
                    }
                });
        }

        ToolsToolBar::ToolsToolBar() :
            _p(new Private)
        {}

        ToolsToolBar::~ToolsToolBar()
        {}

        std::shared_ptr<ToolsToolBar> ToolsToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsToolBar>(new ToolsToolBar);
            out->_init(context, app, actions, parent);
            return out;
        }

        void ToolsToolBar::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ToolsToolBar::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }
    }
}
