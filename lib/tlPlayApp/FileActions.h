// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Action.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! File actions.
        //!
        //! \todo Add actions for opening the next/previous file in the
        //! directory.
        class FileActions : public std::enable_shared_from_this<FileActions>
        {
            DTK_NON_COPYABLE(FileActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            FileActions();

        public:
            ~FileActions();

            static std::shared_ptr<FileActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            DTK_PRIVATE();
        };
    }
}
