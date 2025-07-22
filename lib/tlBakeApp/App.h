// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

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

#include <feather-tk/core/CmdLine.h>
#include <feather-tk/gl/OffscreenBuffer.h>
#include <feather-tk/core/IApp.h>

namespace feather_tk
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
        //! Application command line.
        struct CmdLine
        {
            std::shared_ptr<feather_tk::CmdLineValueArg<std::string> > input;
            std::shared_ptr<feather_tk::CmdLineValueArg<std::string> > output;
            std::shared_ptr<feather_tk::CmdLineValueOption<OTIO_NS::TimeRange> > inOutRange;
            std::shared_ptr<feather_tk::CmdLineValueOption<feather_tk::Size2I> > renderSize;
            std::shared_ptr<feather_tk::CmdLineValueOption<feather_tk::ImageType> > outputPixelType;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioFileName;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioInput;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioDisplay;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioView;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioLook;
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > lutFileName;
            std::shared_ptr<feather_tk::CmdLineValueOption<timeline::LUTOrder> > lutOrder;
            std::shared_ptr<feather_tk::CmdLineValueOption<double> > sequenceDefaultSpeed;
            std::shared_ptr<feather_tk::CmdLineValueOption<int> > sequenceThreadCount;
#if defined(TLRENDER_EXR)
            std::shared_ptr<feather_tk::CmdLineValueOption<exr::Compression> > exrCompression;
            std::shared_ptr<feather_tk::CmdLineValueOption<float> > exrDWACompressionLevel;
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
            std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ffmpegCodec;
            std::shared_ptr<feather_tk::CmdLineValueOption<int> > ffmpegThreadCount;
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            std::shared_ptr<feather_tk::CmdLineValueOption<int> > usdRenderWidth;
            std::shared_ptr<feather_tk::CmdLineValueOption<float> > usdComplexity;
            std::shared_ptr<feather_tk::CmdLineValueOption<usd::DrawMode> > usdDrawMode;
            std::shared_ptr<feather_tk::CmdLineValueOption<bool> > usdEnableLighting;
            std::shared_ptr<feather_tk::CmdLineValueOption<bool> > usdSRGB;
            std::shared_ptr<feather_tk::CmdLineValueOption<size_t> > usdStageCache;
            std::shared_ptr<feather_tk::CmdLineValueOption<size_t> > usdDiskCache;
#endif // TLRENDER_USD
        };

        //! Application.
        class App : public feather_tk::IApp
        {
            FEATHER_TK_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                std::vector<std::string>&);
            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::shared_ptr<feather_tk::Context>&,
                std::vector<std::string>&);

            //! Run the application.
            void run() override;

        private:
            io::Options _getIOOptions() const;

            void _tick();
            void _printProgress();

            CmdLine _cmdLine;
            timeline::OCIOOptions _ocioOptions;
            timeline::LUTOptions _lutOptions;

            std::shared_ptr<timeline::Timeline> _timeline;
            feather_tk::Size2I _renderSize;
            feather_tk::ImageInfo _outputInfo;
            OTIO_NS::TimeRange _timeRange = time::invalidTimeRange;
            OTIO_NS::RationalTime _inputTime = time::invalidTime;
            OTIO_NS::RationalTime _outputTime = time::invalidTime;

            std::shared_ptr<feather_tk::gl::Window> _window;
            std::shared_ptr<io::IPlugin> _usdPlugin;
            std::shared_ptr<timeline::IRender> _render;
            std::shared_ptr<feather_tk::gl::OffscreenBuffer> _buffer;

            std::shared_ptr<io::IWritePlugin> _writerPlugin;
            std::shared_ptr<io::IWrite> _writer;
            std::shared_ptr<feather_tk::Image> _outputImage;

            bool _running = true;
            std::chrono::steady_clock::time_point _startTime;
        };
    }
}
