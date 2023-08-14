// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>
#include <tlCore/Size.h>

struct GLFWwindow;

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace gl
    {
        //! GLFW window options.
        enum class GLFWWindowOptions
        {
            None         = 0,
            Visible      = 1,
            DoubleBuffer = 2
        };

        //! GLFW window wrapper.
        class GLFWWindow : public std::enable_shared_from_this<GLFWWindow>
        {
            TLRENDER_NON_COPYABLE(GLFWWindow);

        protected:
            void _init(
                const std::string& name,
                const math::Size2i&,
                const std::shared_ptr<system::Context>&,
                int options);
            
            GLFWWindow();

        public:
            virtual ~GLFWWindow();

            //! Create a new window.
            static std::shared_ptr<GLFWWindow> create(
                const std::string& name,
                const math::Size2i&,
                const std::shared_ptr<system::Context>&,
                int options =
                    static_cast<int>(GLFWWindowOptions::Visible) |
                    static_cast<int>(GLFWWindowOptions::DoubleBuffer));
        
            //! Get the GLFW window pointer.
            GLFWwindow* getGLFW() const;

            //! Get the window size.
            math::Size2i getSize() const;

            //! Set the window size.
            void setSize(const math::Size2i&);

            //! Show the window.
            void show();

            //! Get whether the window should close.
            bool shouldClose() const;

            //! Get whether the window is in full screen mode.
            bool isFullScreen() const;

            //! Set whether the window is in full screen mode.
            void setFullScreen(bool);

            //! Get whether the window is floating on top.
            bool isFloatOnTop() const;

            //! Set whether the window is floating on top.
            void setFloatOnTop(bool);

            //! Swap the buffers.
            void swap();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
