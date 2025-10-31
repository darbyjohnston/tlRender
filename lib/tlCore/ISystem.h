// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/ISystem.h>
#include <ftk/Core/LogSystem.h>

namespace tl
{
    namespace system
    {
        //! Base class for systems.
        class ISystem : public ftk::ISystem
        {
        protected:
            ISystem(
                const std::shared_ptr<ftk::Context>&,
                const std::string& name);

        public:
            virtual ~ISystem();

        protected:
            void _log(const std::string&, ftk::LogType = ftk::LogType::Message);

        private:
            std::weak_ptr<ftk::LogSystem> _logSystem;
        };
    }
}
