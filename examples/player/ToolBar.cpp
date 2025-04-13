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
                const std::map<std::string, std::shared_ptr<dtk::Action> >& actions)
            {
                dtk::ToolBar::_init(context, dtk::Orientation::Horizontal, nullptr);
                auto tmp = actions;
                addAction(tmp["File/Open"]);
                addAction(tmp["File/Close"]);
                addAction(tmp["File/Reload"]);
            }

            ToolBar::~ToolBar()
            {}

            std::shared_ptr<ToolBar> ToolBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::map<std::string, std::shared_ptr<dtk::Action> >& actions)
            {
                auto out = std::shared_ptr<ToolBar>(new ToolBar);
                out->_init(context, actions);
                return out;
            }
        }
    }
}
