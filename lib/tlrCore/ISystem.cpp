// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/ISystem.h>

#include <tlrCore/Context.h>

namespace tlr
{
    namespace core
    {
        ISystem::ISystem(
            const std::string& name,
            const std::shared_ptr<Context>& context) :
            ICoreSystem(name, context),
            _logSystem(context->getSystem<LogSystem>())
        {
            _logSystem->print(name, "Create");
        }

        ISystem::~ISystem()
        {
            _logSystem->print(_name, "Delete");
        }

        void ISystem::_log(const std::string& value, LogType type)
        {
            _logSystem->print(_name, value, type);
        }
    }
}