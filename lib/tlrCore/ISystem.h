// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/LogSystem.h>

namespace tlr
{
    namespace core
    {
        class LogSystem;

        //! Base class for systems.
        class ISystem : public ICoreSystem
        {
        protected:
            ISystem(
                const std::string& name,
                const std::shared_ptr<Context>&);

        public:
            ~ISystem() override;

        protected:
            void _log(const std::string&, LogType = LogType::Message);

        private:
            std::shared_ptr<LogSystem> _logSystem;
        };
    }
}
