// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlGL/GLFWSystem.h>

#include <tlGL/GL.h>


#include <dtk/core/Context.h>
#include <dtk/core/Format.h>
#include <dtk/core/LogSystem.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace tl
{
    namespace gl
    {
        namespace
        {
            void glfwErrorCallback(int, const char* description)
            {
                std::cerr << "GLFW ERROR: " << description << std::endl;
            }
        }
        
        struct GLFWSystem::Private
        {
            bool glfwInit = false;
        };
        
        GLFWSystem::GLFWSystem(const std::shared_ptr<dtk::Context>& context) :
            ISystem(context, "tl::gl::GLFWSystem"),
            _p(new Private)
        {
            TLRENDER_P();
            
            // Initialize GLFW.
            glfwSetErrorCallback(glfwErrorCallback);
            int glfwMajor = 0;
            int glfwMinor = 0;
            int glfwRevision = 0;
            glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
            _log(dtk::Format("GLFW version: {0}.{1}.{2}").arg(glfwMajor).arg(glfwMinor).arg(glfwRevision));
            if (!glfwInit())
            {
                //! \todo Only log the error for now so that non-OpenGL
                //! tests can run.
                //throw std::runtime_error("Cannot initialize GLFW");
                _log("Cannot initialize GLFW", dtk::LogType::Error);
            }
            p.glfwInit = true;
        }

        GLFWSystem::~GLFWSystem()
        {
            TLRENDER_P();
            if (p.glfwInit)
            {
                glfwTerminate();
            }
        }

        std::shared_ptr<GLFWSystem> GLFWSystem::create(const std::shared_ptr<dtk::Context>& context)
        {
            auto out = context->getSystem<GLFWSystem>();
            if (!out)
            {
                out = std::shared_ptr<GLFWSystem>(new GLFWSystem(context));
                context->addSystem(out);
            }
            return out;
        }
    }
}
