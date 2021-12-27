// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>

#include <tlrGL/FontSystem.h>
#include <tlrGL/Render.h>

#include <tlrCore/TimelinePlayer.h>

struct GLFWwindow;

namespace tlr
{
    class App : public app::IApp
    {
        TLR_NON_COPYABLE(App);

    protected:
        void _init(int argc, char* argv[]);
        App();

    public:
        ~App();

        static std::shared_ptr<App> create(int argc, char* argv[]);

        void run();

        void exit();

    private:
        static void _frameBufferSizeCallback(GLFWwindow*, int, int);
        static void _windowContentScaleCallback(GLFWwindow*, float, float);

        void _tick();

        std::string _input;

        std::shared_ptr<timeline::TimelinePlayer> _timelinePlayer;

        GLFWwindow* _glfwWindow = nullptr;
        std::shared_ptr<imaging::Image> _frameBuffer;
        glm::vec2 _contentScale;
        std::shared_ptr<gl::Render> _render;
        bool _renderDirty = true;
        timeline::VideoData _videoData;

        bool _running = true;
    };
}