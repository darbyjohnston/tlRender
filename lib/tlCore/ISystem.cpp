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
            const std::shared_ptr<ftk::Context>& context,
            const std::string& name) :
            ftk::ISystem(context, name)
        {
            _logSystem = context->getSystem<ftk::LogSystem>();
        }

        ISystem::~ISystem()
        {}

        void ISystem::_log(const std::string& value, ftk::LogType type)
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, value, type);
            }
        }
    }
}
