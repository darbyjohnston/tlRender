// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/ISystem.h>

#include <feather-tk/core/Context.h>

namespace tl
{
    namespace system
    {
        ISystem::ISystem(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::string& name) :
            feather_tk::ISystem(context, name)
        {
            _logSystem = context->getSystem<feather_tk::LogSystem>();
        }

        ISystem::~ISystem()
        {}

        void ISystem::_log(const std::string& value, feather_tk::LogType type)
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, value, type);
            }
        }
    }
}
