// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/Util.h>

namespace tl
{
    //! Timelines
    namespace timeline
    {
        //! Initialize the library.
        void init(const std::shared_ptr<dtk::Context>&);

        //! Timeline system.
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<dtk::Context>&);

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<dtk::Context>&);
        };
    }
}
