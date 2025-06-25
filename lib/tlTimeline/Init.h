// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

namespace tl
{
    //! Timelines
    namespace timeline
    {
        //! Initialize the library.
        void init(const std::shared_ptr<feather_tk::Context>&);

        //! Timeline system.
        class System : public system::ISystem
        {
            FEATHER_TK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<feather_tk::Context>&);

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<feather_tk::Context>&);
        };
    }
}
