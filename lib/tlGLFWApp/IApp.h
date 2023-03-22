// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlUI/IWidget.h>

struct GLFWwindow;

namespace tl
{
    //! GLFW application support.
    namespace glfw
    {
        //! Application options.
        struct Options
        {
            imaging::Size windowSize = imaging::Size(1920, 1080);
            bool fullscreen = false;
        };

        //! Base class for GLFW applications.
        class IApp : public app::IApp
        {
            TLRENDER_NON_COPYABLE(IApp);

        protected:
            void _init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>&,
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<app::ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<app::ICmdLineOption> >& = {});

            IApp();

        public:
            ~IApp();

            //! Run the application.
            virtual void run();

            //! Exit the application.
            void exit();

            //! Add a top-level widget.
            void addWidget(const std::weak_ptr<ui::IWidget>&);

        private:
            void _setFullscreenWindow(bool);
            static void _frameBufferSizeCallback(GLFWwindow*, int, int);
            static void _windowContentScaleCallback(GLFWwindow*, float, float);
            static void _cursorEnterCallback(GLFWwindow*, int);
            static void _cursorPosCallback(GLFWwindow*, double, double);
            static void _mouseButtonCallback(GLFWwindow*, int, int, int);
            static void _keyCallback(GLFWwindow*, int, int, int, int);

            void _tick();

            TLRENDER_PRIVATE();
        };
    }
}
