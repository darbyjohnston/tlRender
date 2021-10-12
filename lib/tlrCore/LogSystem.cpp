// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/LogSystem.h>

#include <mutex>
#include <sstream>

namespace tlr
{
    namespace core
    {
        std::string toString(const LogItem& item)
        {
            std::stringstream ss;
            ss.precision(2);
            switch (item.type)
            {
            case LogType::Message:
                ss << std::fixed << item.time << " " << item.prefix << ": " << item.message;
                break;
            case LogType::Warning:
                ss << std::fixed << item.time << " " << item.prefix << ": Warning: " << item.message;
                break;
            case LogType::Error:
                ss << std::fixed << item.time << " " << item.prefix << ": ERROR: " << item.message;
                break;
            default: break;
            }
            return ss.str();
        }

        struct LogSystem::Private
        {
            std::shared_ptr<observer::Value<LogItem> > log;
            std::chrono::steady_clock::time_point timer;
            std::mutex mutex;
        };

        void LogSystem::_init()
        {
            TLR_PRIVATE_P();
            p.log = observer::Value<LogItem>::create();
            p.timer = std::chrono::steady_clock::now();
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
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> time = now - p.timer;
            std::unique_lock<std::mutex> lock(p.mutex);
            p.log->setAlways({ time.count(), prefix, value, type });
        }

        std::shared_ptr<observer::IValue<LogItem> > LogSystem::observeLog() const
        {
            return _p->log;
        }
    }
}