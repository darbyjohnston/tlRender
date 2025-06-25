// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

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
            //! Application options.
            struct Options
            {
                std::string compareFileName;
                feather_tk::Size2I windowSize = feather_tk::Size2I(1920, 1080);
                bool fullscreen = false;
                bool hud = true;
                timeline::Playback playback = timeline::Playback::Forward;
                OTIO_NS::RationalTime seek = time::invalidTime;
                OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
                timeline::OCIOOptions ocioOptions;
                timeline::LUTOptions lutOptions;
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

                std::string _input;
                Options _options;

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
