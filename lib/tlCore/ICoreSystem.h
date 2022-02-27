// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>
#include <string>

namespace tl
{
    //! Systems.
    namespace system
    {
        class Context;

        //! Base class for core systems.
        class ICoreSystem : public std::enable_shared_from_this<ICoreSystem>
        {
        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<Context>&);
            ICoreSystem();

        public:
            virtual ~ICoreSystem() = 0;

            //! Get the context.
            const std::weak_ptr<Context>& getContext() const;

            //! Get the system name.
            const std::string& getName() const;

        protected:
            std::weak_ptr<Context> _context;
            std::string _name;
        };
    }
}

#include <tlCore/ICoreSystemInline.h>
