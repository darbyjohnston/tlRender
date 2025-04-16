// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "ViewActions.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
        {
            void ViewActions::_init(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto appWeak = std::weak_ptr<App>(app);
            }

            ViewActions::~ViewActions()
            {}

            std::shared_ptr<ViewActions> ViewActions::create(
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<App>& app)
            {
                auto out = std::shared_ptr<ViewActions>(new ViewActions);
                out->_init(context, app);
                return out;
            }

            const std::map<std::string, std::shared_ptr<dtk::Action> >& ViewActions::getActions() const
            {
                return _actions;
            }
        }
    }
}
