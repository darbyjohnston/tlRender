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

#include <ftk/Core/CmdLine.h>
#include <ftk/GL/OffscreenBuffer.h>
#include <ftk/Core/IApp.h>

namespace ftk
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
            std::shared_ptr<ftk::CmdLineValueArg<std::string> > input;
            std::shared_ptr<ftk::CmdLineValueArg<std::string> > output;
            std::shared_ptr<ftk::CmdLineValueOption<OTIO_NS::TimeRange> > inOutRange;
            std::shared_ptr<ftk::CmdLineValueOption<ftk::Size2I> > renderSize;
            std::shared_ptr<ftk::CmdLineValueOption<ftk::ImageType> > outputPixelType;
            std::shared_ptr<ftk::CmdLineValueOption<std::string> > ocioFileName;
            std::shared_ptr<ftk::CmdLineValueOption<std::string> > ocioInput;
            std::shared_ptr<ftk::CmdLineValueOption<std::string> > ocioDisplay;
            std::shared_ptr<ftk::CmdLineValueOption<std::string> > ocioView;
            std::shared_ptr<ftk::CmdLineValueOption<std::string> > ocioLook;
            std::shared_ptr<ftk::CmdLineValueOption<std::string> > lutFileName;
            std::shared_ptr<ftk::CmdLineValueOption<timeline::LUTOrder> > lutOrder;
            std::shared_ptr<ftk::CmdLineValueOption<double> > sequenceDefaultSpeed;
            std::shared_ptr<ftk::CmdLineValueOption<int> > sequenceThreadCount;
#if defined(TLRENDER_EXR)
            std::shared_ptr<ftk::CmdLineValueOption<exr::Compression> > exrCompression;
            std::shared_ptr<ftk::CmdLineValueOption<float> > exrDWACompressionLevel;
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
            std::shared_ptr<ftk::CmdLineValueOption<std::string> > ffmpegCodec;
            std::shared_ptr<ftk::CmdLineValueOption<int> > ffmpegThreadCount;
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            std::shared_ptr<ftk::CmdLineValueOption<int> > usdRenderWidth;
            std::shared_ptr<ftk::CmdLineValueOption<float> > usdComplexity;
            std::shared_ptr<ftk::CmdLineValueOption<usd::DrawMode> > usdDrawMode;
            std::shared_ptr<ftk::CmdLineValueOption<bool> > usdEnableLighting;
            std::shared_ptr<ftk::CmdLineValueOption<bool> > usdSRGB;
            std::shared_ptr<ftk::CmdLineValueOption<size_t> > usdStageCache;
            std::shared_ptr<ftk::CmdLineValueOption<size_t> > usdDiskCache;
#endif // TLRENDER_USD
        };

        //! Application.
        class App : public ftk::IApp
        {
            FTK_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                std::vector<std::string>&);
            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::shared_ptr<ftk::Context>&,
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
            ftk::Size2I _renderSize;
            ftk::ImageInfo _outputInfo;
            OTIO_NS::TimeRange _timeRange = time::invalidTimeRange;
            OTIO_NS::RationalTime _inputTime = time::invalidTime;
            OTIO_NS::RationalTime _outputTime = time::invalidTime;

            std::shared_ptr<ftk::gl::Window> _window;
            std::shared_ptr<io::IPlugin> _usdPlugin;
            std::shared_ptr<timeline::IRender> _render;
            std::shared_ptr<ftk::gl::OffscreenBuffer> _buffer;

            std::shared_ptr<io::IWritePlugin> _writerPlugin;
            std::shared_ptr<io::IWrite> _writer;
            std::shared_ptr<ftk::Image> _outputImage;

            bool _running = true;
            std::chrono::steady_clock::time_point _startTime;
        };
    }
}
