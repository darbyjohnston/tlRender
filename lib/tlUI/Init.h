// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

namespace tl
{
    //! User interface library.
    namespace ui
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);

        //! User interface system.
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            System();

        public:
            ~System() override;

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<system::Context>&);
        };
    }
}
