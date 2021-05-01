// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>

#include <tlrRender/String.h>
#include <tlrRender/StringFormat.h>

#include <iostream>

namespace tlr
{
    namespace app
    {
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
        }
        
        IApp::IApp()
        {}

        IApp::~IApp()
        {}

        int IApp::getExit() const
        {
            return _exit;
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
