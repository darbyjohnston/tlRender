// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlUI/EventLoop.h>
#include <tlUI/Style.h>

#include <tlTimeline/IRender.h>

#include <tlCore/FontSystem.h>

struct GLFWwindow;

namespace tl
{
    namespace examples
    {
        //! Example GLFW user interface application.
        namespace ui_glfw
        {
            class MainWindow;

            //! Application options.
            struct Options
            {
                imaging::Size windowSize = imaging::Size(1920, 1080);
                bool fullscreen = false;
            };

            //! Application.
            class App : public app::IApp
            {
                TLRENDER_NON_COPYABLE(App);

            protected:
                void _init(
                    int argc,
                    char* argv[],
                    const std::shared_ptr<system::Context>&);

                App();

            public:
                ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    int argc,
                    char* argv[],
                    const std::shared_ptr<system::Context>&);

                //! Run the application.
                void run();

                //! Exit the application.
                void exit();

            private:
                void _setFullscreenWindow(bool);
                static void _frameBufferSizeCallback(GLFWwindow*, int, int);
                static void _windowContentScaleCallback(GLFWwindow*, float, float);
                static void _cursorEnterCallback(GLFWwindow*, int);
                static void _cursorPosCallback(GLFWwindow*, double, double);
                static void _mouseButtonCallback(GLFWwindow*, int, int, int);
                static void _keyCallback(GLFWwindow*, int, int, int, int);

                void _tick();

                Options _options;

                GLFWwindow* _glfwWindow = nullptr;
                imaging::Size _windowSize;
                math::Vector2i _windowPos;
                bool _fullscreen = false;
                imaging::Size _frameBufferSize;
                math::Vector2f _contentScale = math::Vector2f(1.F, 1.F);
                std::shared_ptr<imaging::FontSystem> _fontSystem;
                std::shared_ptr<timeline::IRender> _render;

                std::shared_ptr<ui::IconLibrary> _iconLibrary;
                std::shared_ptr<ui::Style> _style;
                std::shared_ptr<ui::EventLoop> _eventLoop;
                std::shared_ptr<MainWindow> _mainWindow;

                bool _running = true;
            };
        }
    }
}
