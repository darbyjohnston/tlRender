// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlUI/EventLoop.h>
#include <tlUI/Style.h>

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

        protected:
            void _setColorConfigOptions(const timeline::ColorConfigOptions&);
            void _setLUTOptions(const timeline::LUTOptions&);

            virtual void _drop(const std::vector<std::string>&);

            virtual void _tick();

            Options _options;

        private:
            void _windowsUpdate(const std::vector<std::shared_ptr<ui::Window> >&);
            void _setActiveWindow(const std::shared_ptr<ui::Window>&);
            void _windowsClose();
            void _windowsDraw();

            TLRENDER_PRIVATE();
        };
    }
}
