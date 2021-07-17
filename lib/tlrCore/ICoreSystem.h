// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>
#include <string>

namespace tlr
{
    namespace core
    {
        class Context;

        //! Base class for core systems.
        class ICoreSystem : public std::enable_shared_from_this<ICoreSystem>
        {
        protected:
            ICoreSystem(
                const std::string& name,
                const std::shared_ptr<Context>&);

        public:
            virtual ~ICoreSystem() = 0;

            //! Get the context.
            const std::shared_ptr<Context>& getContext() const;

            //! Get the system name.
            const std::string& getName() const;

        protected:
            std::shared_ptr<Context> _context;
            std::string _name;
        };
    }
}

#include <tlrCore/ICoreSystemInline.h>
