// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ToolsActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/Tools.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play_app
    {
        struct ToolsActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void ToolsActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            const auto enums = getToolEnums();
            const auto labels = getToolLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto tool = enums[i];
                const dtk::Key shortcut = getShortcut(tool);
                const int shortcutModifier = 0;
                auto action = std::make_shared<dtk::Action>(
                    getText(tool),
                    getIcon(tool),
                    shortcut,
                    shortcutModifier,
                    [appWeak, tool](bool value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto toolsModel = app->getToolsModel();
                            const Tool active = toolsModel->getActiveTool();
                            toolsModel->setActiveTool(tool != active ? tool : Tool::None);
                        }
                    });
                const std::string tooltip = getTooltip(tool);
                if (!tooltip.empty())
                {
                    action->toolTip = dtk::Format(tooltip).
                        arg(dtk::getShortcutLabel(shortcut, shortcutModifier));
                }
                p.actions[labels[i]] = action;
            }
        }

        ToolsActions::ToolsActions() :
            _p(new Private)
        {}

        ToolsActions::~ToolsActions()
        {}

        std::shared_ptr<ToolsActions> ToolsActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<ToolsActions>(new ToolsActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& ToolsActions::getActions() const
        {
            return _p->actions;
        }
    }
}
