// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/Util.h>

namespace tl
{
    namespace gl
    {
        //! GLFW system.
        class GLFWSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(GLFWSystem);

        protected:
            GLFWSystem(const std::shared_ptr<dtk::Context>&);

        public:
            virtual ~GLFWSystem();

            //! Create a new system.
            static std::shared_ptr<GLFWSystem> create(const std::shared_ptr<dtk::Context>&);
        
        private:
            TLRENDER_PRIVATE();
        };
    }
}
