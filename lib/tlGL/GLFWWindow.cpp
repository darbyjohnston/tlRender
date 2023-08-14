// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/GLFWWindow.h>

#include <tlGL/GL.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Vector.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace tl
{
    namespace gl
    {
        namespace
        {
#if defined(TLRENDER_API_GL_4_1_Debug)
            void APIENTRY glDebugOutput(
                GLenum         source,
                GLenum         type,
                GLuint         id,
                GLenum         severity,
                GLsizei        length,
                const GLchar* message,
                const void* userParam)
            {
                switch (severity)
                {
                case GL_DEBUG_SEVERITY_HIGH:
                    std::cerr << "GL HIGH: " << message << std::endl;
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    std::cerr << "GL MEDIUM: " << message << std::endl;
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    std::cerr << "GL LOW: " << message << std::endl;
                    break;
                    //case GL_DEBUG_SEVERITY_NOTIFICATION:
                    //    std::cerr << "GL NOTIFICATION: " << message << std::endl;
                    //    break;
                default: break;
                }
            }
#endif // TLRENDER_API_GL_4_1_Debug
        }

        struct GLFWWindow::Private
        {
            GLFWwindow* glfwWindow = nullptr;
            math::Size2i size;
            math::Vector2i pos;
            bool fullScreen = false;
            bool floatOnTop = false;
        };
        
        void GLFWWindow::_init(
            const std::string& name,
            const math::Size2i& size,
            const std::shared_ptr<system::Context>& context,
            int options)
        {
            TLRENDER_P();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_VISIBLE,
                options & static_cast<int>(GLFWWindowOptions::Visible));
            glfwWindowHint(GLFW_DOUBLEBUFFER,
                options & static_cast<int>(GLFWWindowOptions::DoubleBuffer));
#if defined(TLRENDER_API_GL_4_1_Debug)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // TLRENDER_API_GL_4_1_Debug
            p.glfwWindow = glfwCreateWindow(size.w, size.h, name.c_str(), NULL, NULL);
            if (!p.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }
            glfwMakeContextCurrent(p.glfwWindow);
            if (!gladLoaderLoadGL())
            {
                throw std::runtime_error("Cannot initialize GLAD");
            }
#if defined(TLRENDER_API_GL_4_1_Debug)
            GLint flags = 0;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & static_cast<GLint>(GL_CONTEXT_FLAG_DEBUG_BIT))
            {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(glDebugOutput, _context.get());
                glDebugMessageControl(
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    0,
                    nullptr,
                    GL_TRUE);
            }
#endif // TLRENDER_API_GL_4_1_Debug
            const int glMajor = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
            const int glMinor = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
            const int glRevision = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_REVISION);
            context->log(
                "tl::gl::GLFWWindow",
                string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
        }
        
        GLFWWindow::GLFWWindow() :
            _p(new Private)
        {}

        GLFWWindow::~GLFWWindow()
        {
            TLRENDER_P();
            if (p.glfwWindow)
            {
                glfwDestroyWindow(p.glfwWindow);
            }
        }

        std::shared_ptr<GLFWWindow> GLFWWindow::create(
            const std::string& name,
            const math::Size2i& size,
            const std::shared_ptr<system::Context>& context,
            int options)
        {
            auto out = std::shared_ptr<GLFWWindow>(new GLFWWindow);
            out->_init(name, size, context, options);
            return out;
        }

        GLFWwindow* GLFWWindow::getGLFW() const
        {
            return _p->glfwWindow;
        }

        math::Size2i GLFWWindow::getSize() const
        {
            math::Size2i out;
            glfwGetWindowSize(_p->glfwWindow, &out.w, &out.h);
            return out;
        }

        void GLFWWindow::setSize(const math::Size2i& value)
        {
            glfwSetWindowSize(_p->glfwWindow, value.w, value.h);
        }

        void GLFWWindow::show()
        {
            glfwShowWindow(_p->glfwWindow);
        }

        bool GLFWWindow::shouldClose() const
        {
            return glfwWindowShouldClose(_p->glfwWindow);
        }

        bool GLFWWindow::isFullScreen() const
        {
            return _p->fullScreen;
        }

        void GLFWWindow::setFullScreen(bool value)
        {
            TLRENDER_P();
            if (value == p.fullScreen)
                return;
            p.fullScreen = value;
            if (p.fullScreen)
            {
                glfwGetWindowSize(_p->glfwWindow, &p.size.w, &p.size.h);

                GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
                glfwGetWindowPos(p.glfwWindow, &p.pos.x, &p.pos.y);
                glfwSetWindowMonitor(
                    p.glfwWindow,
                    glfwMonitor,
                    0,
                    0,
                    glfwVidmode->width,
                    glfwVidmode->height,
                    glfwVidmode->refreshRate);
            }
            else
            {
                GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                glfwSetWindowMonitor(
                    p.glfwWindow,
                    NULL,
                    p.pos.x,
                    p.pos.y,
                    p.size.w,
                    p.size.h,
                    0);
            }
        }

        bool GLFWWindow::isFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void GLFWWindow::setFloatOnTop(bool value)
        {
            TLRENDER_P();
            if (value == p.floatOnTop)
                return;
            p.floatOnTop = value;
            glfwSetWindowAttrib(
                p.glfwWindow,
                GLFW_FLOATING,
                p.floatOnTop ? GLFW_TRUE : GLFW_FALSE);
        }

        void GLFWWindow::swap()
        {
            glfwSwapBuffers(_p->glfwWindow);
        }
    }
}
