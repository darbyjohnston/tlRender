// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Models/SettingsModel.h>

#include <dtk/ui/Action.h>

namespace tl
{
    namespace play
    {
        class App;
        class MainWindow;

        //! View actions.
        //!
        //! \todo Add an action for toggling the UI visibility.
        class ViewActions : public std::enable_shared_from_this<ViewActions>
        {
            DTK_NON_COPYABLE(ViewActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            ViewActions();

        public:
            ~ViewActions();

            static std::shared_ptr<ViewActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&);

            const std::vector<dtk::ImageType>& getColorBuffers() const;

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            void _keyShortcutsUpdate(const KeyShortcutsSettings&);

            DTK_PRIVATE();
        };
    }
}
