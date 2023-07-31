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
            std::shared_ptr<observer::ValueObserver<int> > activeObserver;
        };

        void ToolsMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
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
                            auto toolsModel = app->getToolsModel();
                            const int active = toolsModel->getActiveTool();
                            toolsModel->setActiveTool(
                                static_cast<int>(tool) != active ?
                                static_cast<int>(tool) :
                                -1);
                        }
                    });
                addItem(p.items[tool]);
            }

            p.activeObserver = observer::ValueObserver<int>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](int value)
                {
                    for (const auto& item : _p->items)
                    {
                        setItemChecked(
                            item.second,
                            static_cast<int>(item.first) == value);
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
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsMenu>(new ToolsMenu);
            out->_init(app, context, parent);
            return out;
        }
    }
}
