// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <dtk/ui/Action.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;

            //! Window actions.
            class WindowActions : public std::enable_shared_from_this<WindowActions>
            {
                DTK_NON_COPYABLE(WindowActions);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                WindowActions() = default;

            public:
                ~WindowActions();

                static std::shared_ptr<WindowActions> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

            private:
                std::map<std::string, std::shared_ptr<dtk::Action> > _actions;
            };
        }
    }
}
