// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Actions/IActions.h>

namespace tl
{
    namespace play
    {
        class MainWindow;

        //! View actions.
        //!
        //! \todo Add an action for toggling the UI visibility.
        class ViewActions : public IActions
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

            const std::vector<dtk::ImageType>& getColorBuffers() const;

        private:
            std::vector<dtk::ImageType> _colorBuffers;
        };
    }
}
