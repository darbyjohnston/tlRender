// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <tlIO/SequenceIO.h>
#if defined(TLRENDER_EXR)
#include <tlIO/OpenEXR.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
#include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <dtk/gl/OffscreenBuffer.h>

namespace dtk
{
    namespace gl
    {
        class Window;
    }
}

namespace tl
{
    //! tlbake application
    namespace bake
    {
        //! Application options.
        struct Options
        {
            OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
            dtk::Size2I renderSize;
            dtk::ImageType outputPixelType = dtk::ImageType::None;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            float sequenceDefaultSpeed = io::sequenceDefaultSpeed;
            int sequenceThreadCount = io::sequenceThreadCount;

#if defined(TLRENDER_EXR)
            exr::Compression exrCompression = exr::Compression::ZIP;
            float exrDWACompressionLevel = 45.F;
#endif // TLRENDER_EXR

#if defined(TLRENDER_FFMPEG)
            std::string ffmpegWriteProfile;
            int ffmpegThreadCount = ffmpeg::threadCount;
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            int usdRenderWidth = 1920;
            float usdComplexity = 1.F;
            usd::DrawMode usdDrawMode = usd::DrawMode::ShadedSmooth;
            bool usdEnableLighting = true;
            bool usdSRGB = true;
            size_t usdStageCache = 10;
            size_t usdDiskCache = 0;
#endif // TLRENDER_USD
        };

        //! Application.
        class App : public app::BaseApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::vector<std::string>&);
            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::shared_ptr<dtk::Context>&,
                const std::vector<std::string>&);

            //! Run the application.
            int run();

        private:
            io::Options _getIOOptions() const;

            void _tick();
            void _printProgress();

            std::string _input;
            std::string _output;
            Options _options;

            std::shared_ptr<timeline::Timeline> _timeline;
            dtk::Size2I _renderSize;
            dtk::ImageInfo _outputInfo;
            OTIO_NS::TimeRange _timeRange = time::invalidTimeRange;
            OTIO_NS::RationalTime _inputTime = time::invalidTime;
            OTIO_NS::RationalTime _outputTime = time::invalidTime;

            std::shared_ptr<dtk::gl::Window> _window;
            std::shared_ptr<io::IPlugin> _usdPlugin;
            std::shared_ptr<timeline::IRender> _render;
            std::shared_ptr<dtk::gl::OffscreenBuffer> _buffer;

            std::shared_ptr<io::IPlugin> _writerPlugin;
            std::shared_ptr<io::IWrite> _writer;
            std::shared_ptr<dtk::Image> _outputImage;

            bool _running = true;
            std::chrono::steady_clock::time_point _startTime;
        };
    }
}
