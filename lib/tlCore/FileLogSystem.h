// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/ISystem.h>

#include <filesystem>

namespace tl
{
    namespace file
    {
        //! File logging system.
        class FileLogSystem : public dtk::ISystem
        {
            DTK_NON_COPYABLE(FileLogSystem);

        protected:
            FileLogSystem(
                const std::shared_ptr<dtk::Context>&,
                const std::filesystem::path&);

        public:
            virtual ~FileLogSystem();

            //! Create a new system.
            static std::shared_ptr<FileLogSystem> create(
                const std::shared_ptr<dtk::Context>&,
                const std::filesystem::path&);

        private:
            DTK_PRIVATE();
        };
    }
}
