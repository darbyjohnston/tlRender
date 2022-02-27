// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ICoreSystem.h>
#include <tlCore/ValueObserver.h>

#include <chrono>

namespace tl
{
    //! Logging.
    namespace log
    {
        //! Log types.
        enum class Type
        {
            Message,
            Warning,
            Error
        };

        //! Log item.
        struct Item
        {
            float time = 0.F;
            std::string prefix;
            std::string message;
            Type type = Type::Message;
        };

        //! Convert a log item to a string.
        std::string toString(const Item&);

        //! Log system.
        class System : public system::ICoreSystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            System();

        public:
            ~System() override;

            //! Create a new log system.
            static std::shared_ptr<System> create(const std::shared_ptr<system::Context>&);

            //! Print to the log.
            void print(
                const std::string& prefix,
                const std::string&,
                Type = Type::Message);

            //! Observe the log.
            std::shared_ptr<observer::IValue<Item> > observeLog() const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
