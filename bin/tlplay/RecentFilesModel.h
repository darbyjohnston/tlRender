// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/ui/RecentFilesModel.h>
#include <feather-tk/ui/Settings.h>

namespace tl
{
    namespace play
    {
        //! Recent files model.
        class RecentFilesModel : public feather_tk::RecentFilesModel
        {
            FEATHER_TK_NON_COPYABLE(RecentFilesModel);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::Settings>&);

            RecentFilesModel() = default;

        public:
            ~RecentFilesModel();

            //! Create a new model.
            static std::shared_ptr<RecentFilesModel> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::Settings>&);

        private:
            std::shared_ptr<feather_tk::Settings> _settings;
        };
    }
}