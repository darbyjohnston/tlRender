// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "ToolBar.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void ToolBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::ToolBar::_init(context, dtk::Orientation::Horizontal, parent);

                auto tmp = actions;
                addAction(tmp["Open"]);
                addAction(tmp["Close"]);
                addAction(tmp["Reload"]);
            }

            ToolBar::~ToolBar()
            {}

            std::shared_ptr<ToolBar> ToolBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<ToolBar>(new ToolBar);
                out->_init(context, actions, parent);
                return out;
            }
        }
    }
}
