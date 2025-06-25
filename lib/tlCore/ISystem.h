// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/ISystem.h>
#include <feather-tk/core/LogSystem.h>

namespace tl
{
    namespace system
    {
        //! Base class for systems.
        class ISystem : public feather_tk::ISystem
        {
        protected:
            ISystem(
                const std::shared_ptr<feather_tk::Context>&,
                const std::string& name);

        public:
            virtual ~ISystem();

        protected:
            void _log(const std::string&, feather_tk::LogType = feather_tk::LogType::Message);

        private:
            std::weak_ptr<feather_tk::LogSystem> _logSystem;
        };
    }
}
