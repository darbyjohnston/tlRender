// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "ToolBar.h"

#include "FileActions.h"
#include "WindowActions.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void ToolBar::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<IWidget>& parent)
            {
                dtk::ToolBar::_init(context, dtk::Orientation::Horizontal, parent);

                auto actions = fileActions->getActions();
                addAction(actions["Open"]);
                addAction(actions["Close"]);
                addAction(actions["Reload"]);

                actions = windowActions->getActions();
                addAction(actions["FullScreen"]);
            }

            ToolBar::~ToolBar()
            {}

            std::shared_ptr<ToolBar> ToolBar::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<FileActions>& fileActions,
                const std::shared_ptr<WindowActions>& windowActions,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<ToolBar>(new ToolBar);
                out->_init(context, fileActions, windowActions, parent);
                return out;
            }
        }
    }
}
