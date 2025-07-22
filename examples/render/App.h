// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

#include <feather-tk/core/CmdLine.h>
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
    namespace examples
    {
        //! Example rendering application.
        namespace render
        {
            //! Application command line.
            struct CmdLine
            {
                std::shared_ptr<feather_tk::CmdLineValueArg<std::string> > input;
                std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > compareFileName;
                std::shared_ptr<feather_tk::CmdLineValueOption<feather_tk::Size2I> > windowSize;
                std::shared_ptr<feather_tk::CmdLineFlagOption> fullscreen;
                std::shared_ptr<feather_tk::CmdLineFlagOption> hud;
                std::shared_ptr<feather_tk::CmdLineValueOption<timeline::Playback> > playback;
                std::shared_ptr<feather_tk::CmdLineValueOption<OTIO_NS::RationalTime> > seek;
                std::shared_ptr<feather_tk::CmdLineValueOption<OTIO_NS::TimeRange> > inOutRange;
                std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioFileName;
                std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioInput;
                std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioDisplay;
                std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioView;
                std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > ocioLook;
                std::shared_ptr<feather_tk::CmdLineValueOption<std::string> > lutFileName;
                std::shared_ptr<feather_tk::CmdLineValueOption<timeline::LUTOrder> > lutOrder;
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

                void run() override;

            private:
                void _keyCallback(int, int, int, int);

                void _printShortcutsHelp();

                void _tick();

                void _draw();
                void _drawViewport(
                    const feather_tk::Box2I&,
                    uint16_t fontSize,
                    const timeline::CompareOptions&,
                    float rotation);

                void _hudCallback(bool);

                void _playbackCallback(timeline::Playback);

                CmdLine _cmdLine;
                timeline::OCIOOptions _ocioOptions;
                timeline::LUTOptions _lutOptions;

                std::shared_ptr<timeline::Player> _player;

                std::shared_ptr<feather_tk::gl::Window> _window;
                feather_tk::Size2I _frameBufferSize;
                feather_tk::V2F _contentScale = feather_tk::V2F(1.F, 1.F);
                timeline::CompareOptions _compareOptions;
                float _rotation = 0.F;
                bool _hud = false;
                std::shared_ptr<timeline::IRender> _render;
                bool _renderDirty = true;
                std::vector<timeline::VideoData> _videoData;
                std::shared_ptr<feather_tk::ListObserver<timeline::VideoData> > _videoDataObserver;
                std::chrono::steady_clock::time_point _startTime;

                bool _running = true;
            };
        }
    }
}
