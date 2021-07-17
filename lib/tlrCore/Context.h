// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/LogSystem.h>

#include <memory>
#include <vector>

namespace tlr
{
    namespace core
    {
        class ICoreSystem;

        //! Context.
        class Context : public std::enable_shared_from_this<Context>
        {
            TLR_NON_COPYABLE(Context);

        protected:
            void _init();
            Context();

        public:
            ~Context();

            //! Create a new context.
            static std::shared_ptr<Context> create();

            //! Add a system.
            void addSystem(const std::shared_ptr<ICoreSystem>&);

            //! Get the log system.
            const std::shared_ptr<LogSystem>& getLogSystem() const;

            //! Get the log items from initialization.
            std::vector<LogItem> getLogInit();

            //! Get a system.
            template<typename T>
            std::shared_ptr<T> getSystem() const;

            //! Print to the log.
            void log(
                const std::string& prefix,
                const std::string&,
                LogType = LogType::Message);

        private:
            std::shared_ptr<LogSystem> _logSystem;
            std::vector<std::shared_ptr<ICoreSystem> > _systems;
            TLR_PRIVATE();
        };
    }
}

#include <tlrCore/ContextInline.h>
