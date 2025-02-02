// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/Util.h>

namespace tl
{
    //! Qt support
    namespace qt
    {
        //! Surface formats.
        enum class DefaultSurfaceFormat
        {
            None,
            OpenGL_4_1_CoreProfile
        };

        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(
            const std::shared_ptr<dtk::Context>&,
            DefaultSurfaceFormat);

        //! Qt support system.
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            System(
                const std::shared_ptr<dtk::Context>&,
                DefaultSurfaceFormat);

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(
                const std::shared_ptr<dtk::Context>&,
                DefaultSurfaceFormat);
        };
    }
}
