// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/GLFWSystem.h>

#include <tlCore/StringFormat.h>

#include <tlGlad/gl.h>

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
        };
        
        void GLFWSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::gl::GLFWSystem", context);

            // Initialize GLFW.
            glfwSetErrorCallback(glfwErrorCallback);
            int glfwMajor = 0;
            int glfwMinor = 0;
            int glfwRevision = 0;
            glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
            _log(string::Format("GLFW version: {0}.{1}.{2}").arg(glfwMajor).arg(glfwMinor).arg(glfwRevision));
            if (!glfwInit())
            {
                throw std::runtime_error("Cannot initialize GLFW");
            }
        }
        
        GLFWSystem::GLFWSystem() :
            _p(new Private)
        {}

        GLFWSystem::~GLFWSystem()
        {
            glfwTerminate();
        }

        std::shared_ptr<GLFWSystem> GLFWSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<GLFWSystem>(new GLFWSystem);
            out->_init(context);
            return out;
        }
    }
}
