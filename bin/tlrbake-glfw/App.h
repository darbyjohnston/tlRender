// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>

#include <tlrGL/FontSystem.h>
#include <tlrGL/OffscreenBuffer.h>
#include <tlrGL/Render.h>

#include <tlrCore/Timeline.h>

struct GLFWwindow;

namespace tlr
{
    //! Application options.
    struct Options
    {
        int64_t startFrame = -1;
        int64_t endFrame = -1;
        imaging::Size renderSize;
        imaging::PixelType renderPixelType = imaging::PixelType::None;
        imaging::PixelType outputPixelType = imaging::PixelType::None;
    };

    //! Application.
    class App : public app::IApp
    {
        TLR_NON_COPYABLE(App);

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

        std::string _input;
        std::string _output;
        Options _options;

        std::shared_ptr<timeline::Timeline> _timeline;
        io::VideoFrame _videoFrame;
        imaging::Info _renderInfo;
        imaging::Info _outputInfo;
        std::shared_ptr<io::System> _ioSystem;
        std::shared_ptr<io::IWrite> _writer;
        otime::RationalTime _duration;
        otime::TimeRange _range;
        otime::RationalTime _currentTime;

        GLFWwindow* _glfwWindow = nullptr;
        std::shared_ptr<gl::FontSystem> _fontSystem;
        std::shared_ptr<gl::Render> _render;
        std::shared_ptr<gl::OffscreenBuffer> _buffer;
        std::shared_ptr<imaging::Image> _outputImage;

        bool _running = true;
        std::chrono::steady_clock::time_point _startTime;
    };
}
