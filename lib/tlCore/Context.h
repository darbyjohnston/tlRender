// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/LogSystem.h>

#include <memory>
#include <vector>

namespace tl
{
    namespace core
    {
        namespace system
        {
            class ICoreSystem;

            //! Context.
            class Context : public std::enable_shared_from_this<Context>
            {
                TLRENDER_NON_COPYABLE(Context);

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
                const std::shared_ptr<log::System>& getLogSystem() const;

                //! Get the log items from initialization.
                std::vector<log::Item> getLogInit();

                //! Get a system.
                template<typename T>
                std::shared_ptr<T> getSystem() const;

                //! Print to the log.
                void log(
                    const std::string& prefix,
                    const std::string&,
                    log::Type = log::Type::Message);

            private:
                std::shared_ptr<log::System> _logSystem;
                std::vector<std::shared_ptr<ICoreSystem> > _systems;
                TLRENDER_PRIVATE();
            };
        }
    }
}

#include <tlCore/ContextInline.h>
