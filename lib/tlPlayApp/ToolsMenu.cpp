// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ToolsMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/Tools.h>

namespace tl
{
    namespace play_app
    {
        struct ToolsMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<int> > activeObserver;
        };

        void ToolsMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            for (const auto tool : getToolLabels())
            {
                addItem(p.actions[tool]);
            }

            p.activeObserver = dtk::ValueObserver<int>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](int value)
                {
                    const auto enums = getToolEnums();
                    const auto labels = getToolLabels();
                    for (size_t i = 0; i < enums.size(); ++i)
                    {
                        setItemChecked(
                            _p->actions[labels[i]],
                            static_cast<int>(enums[i]) == value);
                    }
                });
        }

        ToolsMenu::ToolsMenu() :
            _p(new Private)
        {}

        ToolsMenu::~ToolsMenu()
        {}

        std::shared_ptr<ToolsMenu> ToolsMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsMenu>(new ToolsMenu);
            out->_init(actions, app, context, parent);
            return out;
        }
    }
}
