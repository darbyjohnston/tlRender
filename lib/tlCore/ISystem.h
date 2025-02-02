// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/ISystem.h>
#include <dtk/core/LogSystem.h>

namespace tl
{
    namespace system
    {
        //! Base class for systems.
        class ISystem : public dtk::ISystem
        {
        protected:
            ISystem(
                const std::shared_ptr<dtk::Context>&,
                const std::string& name);

        public:
            virtual ~ISystem();

        protected:
            void _log(const std::string&, dtk::LogType = dtk::LogType::Message);

        private:
            std::weak_ptr<dtk::LogSystem> _logSystem;
        };
    }
}
