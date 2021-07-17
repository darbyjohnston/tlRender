// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/LogSystem.h>

#include <mutex>

namespace tlr
{
    namespace core
    {
        struct LogSystem::Private
        {
            std::shared_ptr<observer::Value<std::string> > log;
            std::mutex mutex;
        };

        void LogSystem::_init()
        {
            TLR_PRIVATE_P();
            p.log = observer::Value<std::string>::create();
        }

        LogSystem::LogSystem(const std::shared_ptr<Context>& context) :
            ICoreSystem("tlr::core::LogSystem", context),
            _p(new Private)
        {}

        LogSystem::~LogSystem()
        {}

        std::shared_ptr<LogSystem> LogSystem::create(const std::shared_ptr<Context>& context)
        {
            auto out = std::shared_ptr<LogSystem>(new LogSystem(context));
            out->_init();
            return out;
        }

        void LogSystem::print(
            const std::string& prefix,
            const std::string& value,
            LogType type)
        {
            TLR_PRIVATE_P();
            std::string text;
            switch (type)
            {
            case LogType::Message:
                text = "[LOG] " + prefix + ": " + value;
                break;
            case LogType::Warning:
                text = "[LOG] " + prefix + ": " + "Warning: " + value;
                break;
            case LogType::Error:
                text = "[LOG] " + prefix + ": " + "ERROR: " + value;
                break;
            default: break;
            }
            std::unique_lock<std::mutex> lock(p.mutex);
            p.log->setAlways(text);
        }

        std::shared_ptr<observer::IValue<std::string> > LogSystem::observeLog() const
        {
            return _p->log;
        }
    }
}