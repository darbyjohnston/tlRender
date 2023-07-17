// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ToolsMenu.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Tools.h>

namespace tl
{
    namespace play_gl
    {
        struct ToolsMenu::Private
        {
            std::map<Tool, std::shared_ptr<ui::MenuItem> > items;
            std::shared_ptr<observer::MapObserver<Tool, bool> > visibleObserver;
        };

        void ToolsMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            for (const auto tool : getToolEnums())
            {
                p.items[tool] = std::make_shared<ui::MenuItem>(
                    getText(tool),
                    getIcon(tool),
                    getShortcut(tool),
                    0,
                    [this, appWeak, tool](bool value)
                    {
                        close();
                        if (auto app = appWeak.lock())
                        {
                            app->getToolsModel()->setToolVisible(tool, value);
                        }
                    });
                addItem(p.items[tool]);
            }

            p.visibleObserver = observer::MapObserver<Tool, bool>::create(
                app->getToolsModel()->observeToolsVisible(),
                [this](const std::map<Tool, bool>& value)
                {
                    for (const auto i : value)
                    {
                        setItemChecked(_p->items[i.first], i.second);
                    }
                });
        }

        ToolsMenu::ToolsMenu() :
            _p(new Private)
        {}

        ToolsMenu::~ToolsMenu()
        {}

        std::shared_ptr<ToolsMenu> ToolsMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ToolsMenu>(new ToolsMenu);
            out->_init(app, context);
            return out;
        }
    }
}
