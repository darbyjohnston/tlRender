// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Context.h>

#include <tlrCore/AVIOSystem.h>

namespace tlr
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
                });
            _systems.push_back(_logSystem);
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
            std::vector<LogItem> out;
            out.swap(_p->logInit);
            return out;
        }

        void Context::log(const std::string& prefix, const std::string& value, LogType type)
        {
            _logSystem->print(prefix, value, type);
        }
    }
}