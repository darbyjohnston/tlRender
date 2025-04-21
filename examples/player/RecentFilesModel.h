// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/RecentFilesModel.h>
#include <dtk/ui/Settings.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            //! Recent files model.
            class RecentFilesModel : public dtk::RecentFilesModel
            {
                DTK_NON_COPYABLE(RecentFilesModel);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<dtk::Settings>&);

                RecentFilesModel() = default;

            public:
                ~RecentFilesModel();

                //! Create a new model.
                static std::shared_ptr<RecentFilesModel> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<dtk::Settings>&);

            private:
                std::shared_ptr<dtk::Settings> _settings;
            };
        }
    }
}
