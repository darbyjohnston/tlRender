// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/ISystem.h>

#include <tlrCore/Context.h>

namespace tlr
{
    namespace core
    {
        void ISystem::_init(
            const std::string& name,
            const std::shared_ptr<Context>& context)
        {
            ICoreSystem::_init(name, context);

            _logSystem = context->getSystem<LogSystem>();

            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(name, "Create");
            }
        }

        ISystem::ISystem()
        {}

        ISystem::~ISystem()
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, "Delete");
            }
        }

        void ISystem::_log(const std::string& value, LogType type)
        {
            if (auto logSystem = _logSystem.lock())
            {
                logSystem->print(_name, value, type);
            }
        }
    }
}
