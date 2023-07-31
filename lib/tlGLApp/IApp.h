// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlUI/EventLoop.h>
#include <tlUI/Style.h>

#include <tlCore/Image.h>
#include <tlCore/ValueObserver.h>

struct GLFWwindow;

namespace tl
{
    //! OpenGL application support.
    namespace gl
    {
        //! Application options.
        struct Options
        {
            imaging::Size windowSize = imaging::Size(1920, 1080);
            bool fullscreen = false;
        };

        //! Base class for OpenGL applications.
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
            void exit(int = 0);

            //! Get the event loop.
            const std::shared_ptr<ui::EventLoop> getEventLoop() const;

            //! Get the style.
            const std::shared_ptr<ui::Style> getStyle() const;

            //! Get the window size.
            imaging::Size getWindowSize() const;

            //! Set the window size.
            void setWindowSize(const imaging::Size&);

            //! Get whether the window is in full screen mode.
            bool isFullScreen() const;

            //! Observe whether the window is in full screen mode.
            std::shared_ptr<observer::IValue<bool> > observeFullScreen() const;

            //! Set whether the window is in full screen mode.
            void setFullScreen(bool);

        protected:
            void _setCursor(ui::StandardCursor value);
            void _setCursor(
                const std::shared_ptr<imaging::Image>&,
                const math::Vector2i&);

            std::shared_ptr<imaging::Image> _capture(const math::BBox2i&);

            virtual void _drop(const std::vector<std::string>&);

            virtual void _tick();

        private:
            static void _frameBufferSizeCallback(GLFWwindow*, int, int);
            static void _windowContentScaleCallback(GLFWwindow*, float, float);
            static void _cursorEnterCallback(GLFWwindow*, int);
            static void _cursorPosCallback(GLFWwindow*, double, double);
            static void _mouseButtonCallback(GLFWwindow*, int, int, int);
            static void _scrollCallback(GLFWwindow*, double, double);
            static void _keyCallback(GLFWwindow*, int, int, int, int);
            static void _charCallback(GLFWwindow*, unsigned int);
            static void _dropCallback(GLFWwindow*, int, const char**);

            TLRENDER_PRIVATE();
        };
    }
}
