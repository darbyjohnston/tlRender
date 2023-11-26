// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLApp/IApp.h>

#include <tlUI/IClipboard.h>
#include <tlUI/Window.h>

#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tl
{
    namespace gl
    {
        namespace
        {
            class Clipboard : public ui::IClipboard
            {
                TLRENDER_NON_COPYABLE(Clipboard);

            public:
                Clipboard()
                {}

            public:
                virtual ~Clipboard()
                {}

                static std::shared_ptr<Clipboard> create(
                    GLFWwindow* glfwWindow,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<Clipboard>(new Clipboard);
                    out->_init(context);
                    return out;
                }

                void setWindow(GLFWwindow* glfwWindow)
                {
                    _glfwWindow = glfwWindow;
                }

                std::string getText() const override
                {
                    return _glfwWindow ? glfwGetClipboardString(_glfwWindow) : std::string();
                }

                void setText(const std::string& value) override
                {
                    if (_glfwWindow)
                    {
                        glfwSetClipboardString(_glfwWindow, value.c_str());
                    }
                }

            private:
                GLFWwindow* _glfwWindow = nullptr;
            };
        }

        struct IApp::Private
        {
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;

            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<image::FontSystem> fontSystem;
            std::shared_ptr<Clipboard> clipboard;
            std::vector<std::shared_ptr<ui::Window> > windows;
            std::vector<std::shared_ptr<ui::Window> > windowsToRemove;
            bool running = true;

            std::map<std::shared_ptr<ui::Window>, std::shared_ptr<observer::ValueObserver<bool> > > closeObservers;
        };

        void IApp::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<app::ICmdLineArg> >& cmdLineArgs,
            const std::vector<std::shared_ptr<app::ICmdLineOption> >& cmdLineOptions)
        {
            TLRENDER_P();
            if (const GLFWvidmode* monitorMode = glfwGetVideoMode(
                glfwGetPrimaryMonitor()))
            {
                _options.windowSize.w = monitorMode->width * .7F;
                _options.windowSize.h = monitorMode->height * .7F;
            }
            std::vector<std::shared_ptr<app::ICmdLineOption> > cmdLineOptions2 = cmdLineOptions;
            cmdLineOptions2.push_back(
                app::CmdLineValueOption<math::Size2i>::create(
                    _options.windowSize,
                    { "-windowSize", "-ws" },
                    "Window size.",
                    string::Format("{0}x{1}").arg(_options.windowSize.w).arg(_options.windowSize.h)));
            cmdLineOptions2.push_back(
                app::CmdLineFlagOption::create(
                    _options.fullscreen,
                    { "-fullscreen", "-fs" },
                    "Enable full screen mode."));
            app::IApp::_init(
                argv,
                context,
                cmdLineName,
                cmdLineSummary,
                cmdLineArgs,
                cmdLineOptions2);
            if (_exit != 0)
            {
                return;
            }

            p.style = ui::Style::create(_context);
            p.iconLibrary = ui::IconLibrary::create(_context);
            p.fontSystem = context->getSystem<image::FontSystem>();
            p.clipboard = Clipboard::create(nullptr, _context);
        }

        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {}

        int IApp::run()
        {
            TLRENDER_P();
            while (0 == _exit && p.running && !p.windows.empty())
            {
                glfwPollEvents();
                _context->tick();
                _tick();

                ui::TickEvent tickEvent(
                    p.style,
                    p.iconLibrary,
                    p.fontSystem);
                for (const auto& window : p.windows)
                {
                    _tickEvent(
                        window,
                        window->isVisible(false),
                        window->isEnabled(false),
                        tickEvent);
                }

                for (const auto& window : p.windowsToRemove)
                {
                    _removeWindow(window);
                }
                p.windowsToRemove.clear();

                time::sleep(std::chrono::milliseconds(5));
            }
            return _exit;
        }

        void IApp::exit(int r)
        {
            _exit = r;
            _p->running = false;
        }

        const std::shared_ptr<ui::Style> IApp::getStyle() const
        {
            return _p->style;
        }

        int IApp::getScreenCount() const
        {
            int glfwMonitorsCount = 0;
            glfwGetMonitors(&glfwMonitorsCount);
            return glfwMonitorsCount;
        }

        void IApp::addWindow(const std::shared_ptr<ui::Window>& window)
        {
            TLRENDER_P();
            p.windows.push_back(window);
            p.closeObservers[window] = observer::ValueObserver<bool>::create(
                window->observeClose(),
                [this, window](bool value)
                {
                    if (value)
                    {
                        _p->windowsToRemove.push_back(window);
                    }
                });
        }

        void IApp::removeWindow(const std::shared_ptr<ui::Window>& window)
        {
            TLRENDER_P();
            p.windowsToRemove.push_back(window);
        }

        void IApp::_setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            if (value == p.colorConfigOptions)
                return;
            p.colorConfigOptions = value;
        }

        void IApp::_setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
        }

        void IApp::_tick()
        {}

        void IApp::_tickEvent(
            const std::shared_ptr<ui::IWidget>&widget,
            bool visible,
            bool enabled,
            const ui::TickEvent& event)
        {
            TLRENDER_P();
            const bool parentsVisible = visible && widget->isVisible(false);
            const bool parentsEnabled = enabled && widget->isEnabled(false);
            for (const auto& child : widget->getChildren())
            {
                _tickEvent(
                    child,
                    parentsVisible,
                    parentsEnabled,
                    event);
            }
            widget->tickEvent(visible, enabled, event);
        }

        void IApp::_removeWindow(const std::shared_ptr<ui::Window>&window)
        {
            TLRENDER_P();
            const auto i = std::find(p.windows.begin(), p.windows.end(), window);
            if (i != p.windows.end())
            {
                (*i)->setClipboard(nullptr);
                p.windows.erase(i);
            }
            const auto j = p.closeObservers.find(window);
            if (j != p.closeObservers.end())
            {
                p.closeObservers.erase(j);
            }
        }
    }
}
