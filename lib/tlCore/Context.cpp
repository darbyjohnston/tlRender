// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/Context.h>

#include <tlCore/AVIOSystem.h>
#include <tlCore/AudioSystem.h>
#include <tlCore/OS.h>
#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace core
    {
        struct Context::Private
        {
            std::vector<LogItem> logInit;
        };

        void Context::_init()
        {
            _logSystem = LogSystem::create(shared_from_this());
            auto logObserver = observer::ValueObserver<LogItem>::create(
                _logSystem->observeLog(),
                [this](const LogItem& value)
                {
                    _p->logInit.push_back(value);
                },
                observer::CallbackAction::Suppress);
            _systems.push_back(_logSystem);

            const os::SystemInfo info = os::getSystemInfo();
            log("tl::core::Context", string::Format(
                "\n"
                "    System: {0}\n"
                "    Cores:  {1}\n"
                "    RAM:    {2}GB").
                arg(info.name).
                arg(info.cores).
                arg(info.ramGB));

            _systems.push_back(audio::System::create(shared_from_this()));
            _systems.push_back(avio::System::create(shared_from_this()));
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
            _systems.push_back(system);
        }

        std::vector<LogItem> Context::getLogInit()
        {
            return std::move(_p->logInit);
        }

        void Context::log(const std::string& prefix, const std::string& value, LogType type)
        {
            _logSystem->print(prefix, value, type);
        }
    }
}