// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <dtk/ui/MenuBar.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;

            //! Menu bar.
            class MenuBar : public dtk::MenuBar
            {
                DTK_NON_COPYABLE(MenuBar);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                MenuBar() = default;

            public:
                ~MenuBar();

                static std::shared_ptr<MenuBar> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

            private:
                std::map<std::string, std::shared_ptr<dtk::Action> > _actions;
            };
        }
    }
}
