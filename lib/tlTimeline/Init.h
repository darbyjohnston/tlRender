// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlCore/ISystem.h>

namespace tl
{
    //! Timelines
    namespace timeline
    {
        //! Initialize the library.
        void init(const std::shared_ptr<ftk::Context>&);

        //! Timeline system.
        class System : public system::ISystem
        {
            FTK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<ftk::Context>&);

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<ftk::Context>&);
        };
    }
}
