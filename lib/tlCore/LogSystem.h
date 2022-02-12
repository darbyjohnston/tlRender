// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ICoreSystem.h>
#include <tlCore/ValueObserver.h>

#include <chrono>

namespace tl
{
    namespace core
    {
        //! Log types.
        enum class LogType
        {
            Message,
            Warning,
            Error
        };

        //! Log item.
        struct LogItem
        {
            float time = 0.F;
            std::string prefix;
            std::string message;
            LogType type = LogType::Message;
        };

        //! Convert a log item to a string.
        std::string toString(const LogItem&);

        //! Log system.
        class LogSystem : public ICoreSystem
        {
            TLRENDER_NON_COPYABLE(LogSystem);

        protected:
            void _init(const std::shared_ptr<Context>&);
            LogSystem();

        public:
            ~LogSystem() override;

            //! Create a new log system.
            static std::shared_ptr<LogSystem> create(const std::shared_ptr<Context>&);

            //! Print to the log.
            void print(
                const std::string& prefix,
                const std::string&,
                LogType = LogType::Message);

            //! Observe the log.
            std::shared_ptr<observer::IValue<LogItem> > observeLog() const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
