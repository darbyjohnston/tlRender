// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "WindowActions.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void WindowActions::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto appWeak = std::weak_ptr<App>(app);
            }

            WindowActions::~WindowActions()
            {}

            std::shared_ptr<WindowActions> WindowActions::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto out = std::shared_ptr<WindowActions>(new WindowActions);
                out->_init(context, app);
                return out;
            }

            const std::map<std::string, std::shared_ptr<dtk::Action> >& WindowActions::getActions() const
            {
                return _actions;
            }
        }
    }
}
