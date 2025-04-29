// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/FrameMenu.h>

#include <tlPlayApp/Actions/FrameActions.h>

namespace tl
{
    namespace play
    {
        void FrameMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FrameActions>& frameActions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);

            auto actions = frameActions->getActions();
            addAction(actions["Start"]);
            addAction(actions["End"]);
            addDivider();
            addAction(actions["Prev"]);
            addAction(actions["PrevX10"]);
            addAction(actions["PrevX100"]);
            addDivider();
            addAction(actions["Next"]);
            addAction(actions["NextX10"]);
            addAction(actions["NextX100"]);
            addDivider();
            addAction(actions["FocusCurrent"]);
        }

        FrameMenu::~FrameMenu()
        {}

        std::shared_ptr<FrameMenu> FrameMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FrameActions>& frameActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FrameMenu>(new FrameMenu);
            out->_init(context, frameActions, parent);
            return out;
        }
    }
}
