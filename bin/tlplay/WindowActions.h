// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/ui/Action.h>

namespace tl
{
    namespace play
    {
        class App;
        class MainWindow;

        //! Window actions.
        class WindowActions : public std::enable_shared_from_this<WindowActions>
        {
            FEATHER_TK_NON_COPYABLE(WindowActions);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            WindowActions() = default;

        public:
            ~WindowActions();

            static std::shared_ptr<WindowActions> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            const std::map<std::string, std::shared_ptr<feather_tk::Action> >& getActions() const;

        private:
            std::map<std::string, std::shared_ptr<feather_tk::Action> > _actions;
            std::shared_ptr<feather_tk::ValueObserver<bool> > _fullScreenObserver;
        };
    }
}