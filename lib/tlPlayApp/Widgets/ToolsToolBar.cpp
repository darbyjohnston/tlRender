// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/ToolsToolBar.h>

#include <tlPlayApp/Models/ToolsModel.h>

namespace tl
{
    namespace play
    {
        void ToolsToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, dtk::Orientation::Horizontal, parent);
            auto tools = getToolsInToolbar();
            auto tmp = actions;
            for (const auto tool : tools)
            {
                addAction(tmp[getLabel(tool)]);
            }
        }

        ToolsToolBar::ToolsToolBar()
        {}

        ToolsToolBar::~ToolsToolBar()
        {}

        std::shared_ptr<ToolsToolBar> ToolsToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsToolBar>(new ToolsToolBar);
            out->_init(context, actions, parent);
            return out;
        }
    }
}
