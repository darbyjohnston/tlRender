// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlCore/ISystem.h>

#include <ftk/Core/Context.h>

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
