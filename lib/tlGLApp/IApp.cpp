// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLApp/IApp.h>

#include <tlGLApp/Window.h>

#include <tlUI/IClipboard.h>

#include <tlGL/GLFWWindow.h>

#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tl
{
    namespace gl_app
    {
        namespace
        {
            const size_t tickTimeout = 5;

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
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<image::FontSystem> fontSystem;
            std::shared_ptr<Clipboard> clipboard;
            std::vector<std::shared_ptr<Window> > windows;
            std::shared_ptr<Window> clipboardWindow;
            std::vector<std::shared_ptr<Window> > windowsToRemove;
            bool running = true;

            std::map<std::shared_ptr<Window>, std::shared_ptr<observer::ValueObserver<bool> > > closeObservers;
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
                const auto t0 = std::chrono::steady_clock::now();

                glfwPollEvents();

                // Tick the various objects.
                _context->tick();
                _tick();
                ui::TickEvent tickEvent(
                    p.style,
                    p.iconLibrary,
                    p.fontSystem);
                for (const auto& window : p.windows)
                {
                    _tickRecursive(
                        window,
                        window->isVisible(false),
                        window->isEnabled(false),
                        tickEvent);
                }

                // Remove closed windows
                for (const auto& window : p.windowsToRemove)
                {
                    _removeWindow(window);
                }
                p.windowsToRemove.clear();

                // Sleep for a bit.
                const auto t1 = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = t1 - t0;
                const float diffClamped = math::clamp(
                    diff.count() * 1000.F,
                    0.F,
                    static_cast<float>(tickTimeout));
                const size_t sleep = tickTimeout - diffClamped;
                time::sleep(std::chrono::milliseconds(sleep));
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

        void IApp::addWindow(const std::shared_ptr<Window>& window)
        {
            TLRENDER_P();
            window->setClipboard(p.clipboard);
            p.windows.push_back(window);

            p.clipboardWindow = window;
            p.clipboard->setWindow(window->getGLFWWindow()->getGLFW());

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

        void IApp::removeWindow(const std::shared_ptr<Window>& window)
        {
            TLRENDER_P();
            p.windowsToRemove.push_back(window);
        }

        void IApp::_tick()
        {}

        void IApp::_tickRecursive(
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
                _tickRecursive(
                    child,
                    parentsVisible,
                    parentsEnabled,
                    event);
            }
            widget->tickEvent(visible, enabled, event);
        }

        void IApp::_removeWindow(const std::shared_ptr<Window>&window)
        {
            TLRENDER_P();
            const auto i = std::find(p.windows.begin(), p.windows.end(), window);
            if (i != p.windows.end())
            {
                if (*i == p.clipboardWindow)
                {
                    p.clipboard->setWindow(nullptr);
                    p.clipboardWindow.reset();
                }
                p.windows.erase(i);
            }
            const auto j = p.closeObservers.find(window);
            if (j != p.closeObservers.end())
            {
                p.closeObservers.erase(j);
            }
            if (!p.clipboardWindow && !p.windows.empty())
            {
                p.clipboardWindow = p.windows.front();
                p.clipboard->setWindow(p.clipboardWindow->getGLFWWindow()->getGLFW());
            }
        }
    }
}
