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
            class MainWindow;

            //! View actions.
            class ViewActions : public std::enable_shared_from_this<ViewActions>
            {
                DTK_NON_COPYABLE(ViewActions);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<MainWindow>&);

                ViewActions() = default;

            public:
                ~ViewActions();

                static std::shared_ptr<ViewActions> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<MainWindow>&);

                const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

            private:
                std::map<std::string, std::shared_ptr<dtk::Action> > _actions;
                std::shared_ptr<dtk::ValueObserver<bool> > _frameObserver;
            };
        }
    }
}
