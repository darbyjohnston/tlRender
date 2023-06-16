// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! Qt Quick support library.
    namespace qtquick
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(const std::shared_ptr<system::Context>&);

        //! Qt Quick support system.
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

        //! Get the context singleton.
        //!
        //! \todo What's a better way to get the context to QML objects?
        const std::shared_ptr<system::Context>& getContext();
    }
}
