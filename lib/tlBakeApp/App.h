// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <tlIO/IO.h>

struct GLFWwindow;

namespace tl
{
    //! Bake application.
    namespace bake
    {
        //! Application options.
        struct Options
        {
            otime::TimeRange inOutRange = time::invalidTimeRange;
            imaging::Size renderSize;
            imaging::PixelType outputPixelType = imaging::PixelType::None;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
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

        private:
            void _tick();
            void _printProgress();

            std::string _input;
            std::string _output;
            Options _options;

            std::shared_ptr<timeline::Timeline> _timeline;
            imaging::Size _renderSize;
            imaging::Info _outputInfo;
            otime::RationalTime _duration = time::invalidTime;
            otime::TimeRange _range = time::invalidTimeRange;
            otime::RationalTime _currentTime = time::invalidTime;

            GLFWwindow* _glfwWindow = nullptr;
            std::shared_ptr<timeline::IRender> _render;
            std::shared_ptr<gl::OffscreenBuffer> _buffer;

            std::shared_ptr<io::IPlugin> _writerPlugin;
            std::shared_ptr<io::IWrite> _writer;
            std::shared_ptr<imaging::Image> _outputImage;

            bool _running = true;
            std::chrono::steady_clock::time_point _startTime;
        };
    }
}
