// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/Init.h>

#include <tlGL/GL.h>
#if defined(TLRENDER_GLFW)
#include <tlGL/GLFWSystem.h>
#endif // TLRENDER_GLFW

#include <tlCore/Context.h>

namespace tl
{
    namespace gl
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
#if defined(TLRENDER_GLFW)
            if (!context->getSystem<GLFWSystem>())
            {
                context->addSystem(GLFWSystem::create(context));
            }
#endif // TLRENDER_GLFW
        }

        void initGLAD()
        {
            gladLoaderLoadGL();
        }
    }
}
