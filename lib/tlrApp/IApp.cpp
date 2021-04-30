// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>

#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

#include <glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace tlr
{
    namespace app
    {
        namespace
        {
            void glfwErrorCallback(int, const char* description)
            {
                std::cerr << "GLFW ERROR: " << description << std::endl;
            }

            /*void APIENTRY glDebugOutput(
                GLenum         source,
                GLenum         type,
                GLuint         id,
                GLenum         severity,
                GLsizei        length,
                const GLchar * message,
                const void *   userParam)
            {
                switch (severity)
                {
                case GL_DEBUG_SEVERITY_HIGH_KHR:
                case GL_DEBUG_SEVERITY_MEDIUM_KHR:
                    std::cerr << "DEBUG: " << message << std::endl;
                    break;
                default: break;
                }
            }*/
        }

        void IApp::_init(
            int argc,
            char* argv[],
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<ICmdLineArg> >& args,
            const std::vector<std::shared_ptr<ICmdLineOption> >& options)
        {
            // Parse the command line.
            for (int i = 1; i < argc; ++i)
            {
                _cmdLine.push_back(argv[i]);
            }
            _cmdLineName = cmdLineName;
            _cmdLineSummary = cmdLineSummary;
            _cmdLineArgs = args;
            _cmdLineOptions = options;
            _cmdLineOptions.push_back(CmdLineValueOption<size_t>::create(
                _options.ioVideoQueueSize,
                { "-ioVideoQueueSize", "-vqs" },
                string::Format("Set the video queue size. Default: {0}").
                    arg(_options.ioVideoQueueSize),
                "(value)"));
            _cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.verbose,
                { "-verbose", "-v" },
                "Enable verbose mode."));
            _cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.help,
                { "-help", "-h", "--help", "--h" },
                "Show this message."));
            _exit = _parseCmdLine();

            // Create the I/O system.
            _ioSystem = av::io::System::create();
            _ioSystem->setVideoQueueSize(IApp::_options.ioVideoQueueSize);

            // Initialize GLFW.
            glfwSetErrorCallback(glfwErrorCallback);
            int glfwMajor = 0;
            int glfwMinor = 0;
            int glfwRevision = 0;
            glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
            {
                std::stringstream ss;
                ss << "GLFW version: " << glfwMajor << "." << glfwMinor << "." << glfwRevision;
                _printVerbose(ss.str());
            }
            if (!glfwInit())
            {
                throw std::runtime_error("Cannot initialize GLFW");
            }
        }
        
        IApp::IApp()
        {}

        IApp::~IApp()
        {
            _destroyWindow();
            glfwTerminate();
        }

        int IApp::getExit() const
        {
            return _exit;
        }

        void IApp::_createWindow(const imaging::Size& value)
        {
            _windowSize = value;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
            //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            _glfwWindow = glfwCreateWindow(
                _windowSize.w,
                _windowSize.h,
                "tlrplay",
                NULL,
                NULL);
            if (!_glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }
            glfwSetWindowUserPointer(_glfwWindow, this);
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(_glfwWindow, &width, &height);
            _frameBufferSize.w = width;
            _frameBufferSize.h = height;
            glfwGetWindowContentScale(_glfwWindow, &_contentScale.x, &_contentScale.y);

            glfwMakeContextCurrent(_glfwWindow);
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                throw std::runtime_error("Cannot initialize GLAD");
            }
            /*GLint flags = 0;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & static_cast<GLint>(GL_CONTEXT_FLAG_DEBUG_BIT))
            {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(glDebugOutput, context.get());
                glDebugMessageControl(
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    0,
                    nullptr,
                    GLFW_TRUE);
            }*/
            const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
            const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
            const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
            {
                std::stringstream ss;
                ss << "OpenGL version: " << glMajor << "." << glMinor << "." << glRevision;
                _printVerbose(ss.str());
            }

            _fontSystem = render::FontSystem::create();
            _render = render::Render::create();
        }

        void IApp::_destroyWindow()
        {
            _render.reset();
            _fontSystem.reset();
            if (_glfwWindow)
            {
                glfwDestroyWindow(_glfwWindow);
            }
        }

        void IApp::_print(const std::string& value)
        {
            std::cout << value << std::endl;
        }

        void IApp::_printVerbose(const std::string& value)
        {
            if (_options.verbose)
            {
                std::cout << value << std::endl;
            }
        }

        void IApp::_printError(const std::string& value)
        {
            std::cerr << "ERROR: " << value << std::endl;
        }

        int IApp::_parseCmdLine()
        {
            for (const auto& i : _cmdLineOptions)
            {
                try
                {
                    i->parse(_cmdLine);
                }
                catch (const std::exception& e)
                {
                    std::stringstream ss;
                    ss << "Cannot parse option \"" << i->getName() << "\": " << e.what();
                    throw std::runtime_error(ss.str());
                }
            }
            if (_cmdLine.size() != _cmdLineArgs.size() || _options.help)
            {
                _printCmdLineHelp();
                return 1;
            }
            for (const auto& i : _cmdLineArgs)
            {
                try
                {
                    i->parse(_cmdLine);
                }
                catch (const std::exception& e)
                {
                    std::stringstream ss;
                    ss << "Cannot parse argument \"" << i->getName() << "\": " << e.what();
                    throw std::runtime_error(ss.str());
                }
            }
            return 0;
        }

        void IApp::_printCmdLineHelp()
        {
            _print("\n" + _cmdLineName + "\n");
            _print("    " + _cmdLineSummary + "\n");
            _print("Usage:\n");
            {
                std::stringstream ss;
                ss << "    " + _cmdLineName;
                if (_cmdLineArgs.size())
                {
                    std::vector<std::string> args;
                    for (const auto& i : _cmdLineArgs)
                    {
                        args.push_back("(" + string::toLower(i->getName()) + ")");
                    }
                    ss << " " << string::join(args, " ");
                }
                if (_cmdLineOptions.size())
                {
                    ss << " [option],...";
                }
                ss << std::endl;
                _print(ss.str());
            }
            _print("Arguments:\n");
            for (const auto& i : _cmdLineArgs)
            {
                std::stringstream ss;
                ss << "    " << i->getName() << " - " << i->getHelp() << std::endl;
                _print(ss.str());
            }
            _print("Options:\n");
            for (const auto& i : _cmdLineOptions)
            {
                std::stringstream ss;
                ss << "    " << string::join(i->getNames(), "|");
                const std::string argsHelp = i->getArgsHelp();
                if (!argsHelp.empty())
                {
                    ss << " " << argsHelp;
                }
                ss << " - " << i->getHelp() << std::endl;
                _print(ss.str());
            }
        }
    }
}
