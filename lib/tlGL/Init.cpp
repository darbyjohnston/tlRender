// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/Init.h>

#include <tlGL/GLFWSystem.h>

#include <tlCore/Context.h>

#include <tlGlad/gl.h>

namespace tl
{
    namespace gl
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            if (!context->getSystem<GLFWSystem>())
            {
                context->addSystem(GLFWSystem::create(context));
            }
        }

        void initGLAD()
        {
            gladLoaderLoadGL();
        }
    }
}
