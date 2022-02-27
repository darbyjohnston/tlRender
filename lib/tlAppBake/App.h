// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlTimeline/SoftwareRender.h>
#include <tlTimeline/Timeline.h>

#include <tlIO/IO.h>

#include <tlCore/FontSystem.h>
#include <tlCore/OCIO.h>

namespace tl
{
    namespace app
    {
        //! Functionality for the tlRender bake application.
        namespace bake
        {
            //! Application options.
            struct Options
            {
                int64_t startFrame = -1;
                int64_t endFrame = -1;
                core::imaging::Size renderSize;
                core::imaging::PixelType outputPixelType = core::imaging::PixelType::None;
                core::imaging::ColorConfig colorConfig;
            };

            //! Application.
            class App : public app::IApp
            {
                TLRENDER_NON_COPYABLE(App);

            protected:
                void _init(int argc, char* argv[]);
                App();

            public:
                ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(int argc, char* argv[]);

                //! Run the application.
                void run();

            private:
                void _tick();
                void _printProgress();

                std::string _input;
                std::string _output;
                Options _options;

                std::shared_ptr<timeline::Timeline> _timeline;
                core::imaging::Size _renderSize;
                core::imaging::Info _outputInfo;
                otime::RationalTime _duration = core::time::invalidTime;
                otime::TimeRange _range = core::time::invalidTimeRange;
                otime::RationalTime _currentTime = core::time::invalidTime;

                std::shared_ptr<core::imaging::FontSystem> _fontSystem;
                std::shared_ptr<timeline::SoftwareRender> _render;

                std::shared_ptr<io::IPlugin> _writerPlugin;
                std::shared_ptr<io::IWrite> _writer;
                std::shared_ptr<core::imaging::Image> _outputImage;

                bool _running = true;
                std::chrono::steady_clock::time_point _startTime;
            };
        }
    }
}
