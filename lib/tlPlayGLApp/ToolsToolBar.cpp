// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ToolsToolBar.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Tools.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct ToolsToolBar::Private
        {
            std::shared_ptr<ui::ButtonGroup> buttonGroup;
            std::map<Tool, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;
            std::shared_ptr<observer::ValueObserver<int> > activeObserver;
        };

        void ToolsToolBar::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                "tl::play_gl::ToolsToolBar",
                context,
                parent);
            TLRENDER_P();

            p.buttonGroup = ui::ButtonGroup::create(ui::ButtonGroupType::Toggle, context);
            for (const auto tool : toolsInToolbar())
            {
                auto button = ui::ToolButton::create(context);
                button->setIcon(getIcon(tool));
                button->setCheckable(true);
                p.buttonGroup->addButton(button);
                p.buttons[tool] = button;
            }

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            for (const auto& button : p.buttons)
            {
                button.second->setParent(p.layout);
            }

            auto appWeak = std::weak_ptr<App>(app);
            p.buttonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getToolsModel()->setActiveTool(value ? index : -1);
                    }
                });

            p.activeObserver = observer::ValueObserver<int>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](int value)
                {
                    for (const auto& button : _p->buttons)
                    {
                        button.second->setChecked(static_cast<int>(button.first) == value);
                    }
                });
        }

        ToolsToolBar::ToolsToolBar() :
            _p(new Private)
        {}

        ToolsToolBar::~ToolsToolBar()
        {}

        std::shared_ptr<ToolsToolBar> ToolsToolBar::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsToolBar>(new ToolsToolBar);
            out->_init(app, context, parent);
            return out;
        }

        void ToolsToolBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ToolsToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
