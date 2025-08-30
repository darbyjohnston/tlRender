// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/ISystem.h>

#include <filesystem>

namespace tl
{
    namespace file
    {
        //! File logging system.
        class FileLogSystem : public ftk::ISystem
        {
            FTK_NON_COPYABLE(FileLogSystem);

        protected:
            FileLogSystem(
                const std::shared_ptr<ftk::Context>&,
                const std::filesystem::path&);

        public:
            virtual ~FileLogSystem();

            //! Create a new system.
            static std::shared_ptr<FileLogSystem> create(
                const std::shared_ptr<ftk::Context>&,
                const std::filesystem::path&);

        private:
            FTK_PRIVATE();
        };
    }
}
