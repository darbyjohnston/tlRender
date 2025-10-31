// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/ISystem.h>

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
