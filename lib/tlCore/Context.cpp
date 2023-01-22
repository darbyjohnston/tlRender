// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/Context.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/OS.h>
#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace system
    {
        struct Context::Private
        {
        };

        void Context::_init()
        {
            _logSystem = log::System::create(shared_from_this());
            addSystem(_logSystem);

            const os::SystemInfo info = os::getSystemInfo();
            log("tl::system::Context", string::Format(
                "\n"
                "    System: {0}\n"
                "    Cores:  {1}\n"
                "    RAM:    {2}GB").
                arg(info.name).
                arg(info.cores).
                arg(info.ramGB));

            addSystem(audio::System::create(shared_from_this()));
        }

        Context::Context() :
            _p(new Private)
        {}

        Context::~Context()
        {}

        std::shared_ptr<Context> Context::create()
        {
            auto out = std::shared_ptr<Context>(new Context);
            out->_init();
            return out;
        }

        void Context::addSystem(const std::shared_ptr<ICoreSystem>& system)
        {
            _systems[system] = std::chrono::steady_clock::now();
        }

        void Context::log(const std::string& prefix, const std::string& value, log::Type type)
        {
            _logSystem->print(prefix, value, type);
        }

        void Context::tick()
        {
            const auto now = std::chrono::steady_clock::now();
            for (auto& i : _systems)
            {
                const auto tickTime = i.first->getTickTime();
                if (tickTime > std::chrono::milliseconds(0) &&
                    (i.second + i.first->getTickTime()) <= now)
                {
                    i.first->tick();
                    i.second = now;
                }
            }
        }
    }
}
