// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <dtk/core/ObservableList.h>

#include <string>

namespace dtk
{
    class Context;
}

namespace tl
{
    namespace ui
    {
        //! File system drives model.
        class DrivesModel : public std::enable_shared_from_this<DrivesModel>
        {
            TLRENDER_NON_COPYABLE(DrivesModel);

        protected:
            void _init(const std::shared_ptr<dtk::Context>&);

            DrivesModel();

        public:
            ~DrivesModel();

            //! Create a new model.
            static std::shared_ptr<DrivesModel> create(
                const std::shared_ptr<dtk::Context>&);

            //! Observe the list of drives.
            std::shared_ptr<dtk::IObservableList<std::string> > observeDrives() const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
