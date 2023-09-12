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
            math::Size2i windowSize = math::Size2i(1920, 1080);
            bool fullscreen = false;
        };

        //! Base class for OpenGL applications.
        class IApp : public app::IApp
        {
            TLRENDER_NON_COPYABLE(IApp);

        protected:
            void _init(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&,
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<app::ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<app::ICmdLineOption> >& = {});

            IApp();

        public:
            ~IApp();

            //! Run the application.
            virtual int run();

            //! Exit the application.
            void exit(int = 0);

            //! Get the event loop.
            const std::shared_ptr<ui::EventLoop> getEventLoop() const;

            //! Get the style.
            const std::shared_ptr<ui::Style> getStyle() const;

            //! Get the window size.
            math::Size2i getWindowSize() const;

            //! Set the window size.
            void setWindowSize(const math::Size2i&);

            //! Get whether the window is in full screen mode.
            bool isFullScreen() const;

            //! Observe whether the window is in full screen mode.
            std::shared_ptr<observer::IValue<bool> > observeFullScreen() const;

            //! Set whether the window is in full screen mode.
            void setFullScreen(bool);

            //! Get whether the window is floating on top.
            bool isFloatOnTop() const;

            //! Observe whether the window is floating on top.
            std::shared_ptr<observer::IValue<bool> > observeFloatOnTop() const;

            //! Set whether the window is floating on top.
            void setFloatOnTop(bool);

        protected:
            void _setColorConfigOptions(const timeline::ColorConfigOptions&);
            void _setLUTOptions(const timeline::LUTOptions&);

            std::shared_ptr<OffscreenBuffer> _capture(const math::Box2i&);

            virtual void _drop(const std::vector<std::string>&);

            virtual void _tick();

            Options _options;

        private:
            void _buttonCallback(int, int, int);
            void _scrollCallback(const math::Vector2f&);
            void _keyCallback(int, int, int, int);
            void _charCallback(unsigned int);
            void _dropCallback(int, const char**);
            
            TLRENDER_PRIVATE();
        };
    }
}
