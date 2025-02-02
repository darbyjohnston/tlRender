// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/ISystem.h>

#include <dtk/core/Context.h>

namespace tl
{
    namespace system
    {
        ISystem::ISystem(
            const std::shared_ptr<dtk::Context>& context,
            const std::string& name) :
            dtk::ISystem(context, name)
        {
            _logSystem = context->getSystem<dtk::LogSystem>();

            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(name, "Create");
            }
        }

        ISystem::~ISystem()
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, "Delete");
            }
        }

        void ISystem::_log(const std::string& value, dtk::LogType type)
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, value, type);
            }
        }
    }
}
