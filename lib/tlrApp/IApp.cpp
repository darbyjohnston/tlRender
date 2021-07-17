// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>

#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>
#if defined(FFmpeg_FOUND)
#include <tlrCore/FFmpeg.h>
#endif

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
            // Create the context.
            _context = core::Context::create();

            // Parse the command line.
            for (int i = 1; i < argc; ++i)
            {
                _cmdLine.push_back(argv[i]);
            }
            _cmdLineName = cmdLineName;
            _cmdLineSummary = cmdLineSummary;
            _cmdLineArgs = args;
            _cmdLineOptions = options;
            _cmdLineOptions.push_back(CmdLineValueOption<float>::create(
                _options.seqDefaultSpeed,
                { "-seqDefaultSpeed" },
                "Default speed for image sequences.",
                string::Format("{0}").arg(_options.seqDefaultSpeed)));
            _cmdLineOptions.push_back(CmdLineValueOption<int>::create(
                _options.seqThreadCount,
                { "-seqThreadCount" },
                "Number of threads for image sequence I/O.",
                string::Format("{0}").arg(_options.seqThreadCount)));
#if defined(FFmpeg_FOUND)
            _cmdLineOptions.push_back(CmdLineValueOption<int>::create(
                _options.ffThreadCount,
                { "-ffThreadCount" },
                "Number of threads for FFmpeg I/O.",
                string::Format("{0}").arg(_options.ffThreadCount)));
            _cmdLineOptions.push_back(app::CmdLineValueOption<std::string>::create(
                _options.ffWriteProfile,
                { "-ffProfile", "-ffp" },
                "FFmpeg output profile.",
                std::string(),
                string::join(ffmpeg::getProfileLabels(), ", ")));
#endif
            _cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.verbose,
                { "-verbose", "-v" },
                "Enable verbose mode."));
            _cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.help,
                { "-help", "-h", "--help", "--h" },
                "Show this message."));
            _exit = _parseCmdLine();

            // Set AV I/O options.
            avio::Options avioOptions;
            {
                std::stringstream ss;
                ss << _options.seqDefaultSpeed;
                avioOptions["SequenceIO/DefaultSpeed"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.seqThreadCount;
                avioOptions["SequenceIO/ThreadCount"] = ss.str();
            }
            if (!_options.ffWriteProfile.empty())
            {
                avioOptions["ffmpeg/WriteProfile"] = _options.ffWriteProfile;
            }
            {
                std::stringstream ss;
                ss << _options.ffThreadCount;
                avioOptions["ffmpeg/ThreadCount"] = ss.str();
            }
            _context->getAVIOSystem()->setOptions(avioOptions);
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

        void IApp::_printNewline()
        {
            std::cout << std::endl;
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
                    throw std::runtime_error(string::Format("Cannot parse option \"{0}\": {1}").
                        arg(i->getMatchedName()).
                        arg(e.what()));
                }
            }
            size_t requiredArgs = 0;
            size_t optionalArgs = 0;
            for (const auto& i : _cmdLineArgs)
            {
                if (!i->isOptional())
                {
                    ++requiredArgs;
                }
                else
                {
                    ++optionalArgs;
                }
            }
            if (_cmdLine.size() < requiredArgs ||
                _cmdLine.size() > requiredArgs + optionalArgs ||
                _options.help)
            {
                _printCmdLineHelp();
                return 1;
            }
            for (const auto& i : _cmdLineArgs)
            {
                try
                {
                    if (!(_cmdLine.empty() && i->isOptional()))
                    {
                        i->parse(_cmdLine);
                    }
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(string::Format("Cannot parse argument \"{0}\": {1}").
                        arg(i->getName()).
                        arg(e.what()));
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
                        const bool optional = i->isOptional();
                        args.push_back(
                            (optional ? "[" : "(") +
                            string::toLower(i->getName()) +
                            (optional ? "]" : ")"));
                    }
                    ss << " " << string::join(args, " ");
                }
                if (_cmdLineOptions.size())
                {
                    ss << " [option],...";
                }
                _print(ss.str());
                _printNewline();
            }
            _print("Arguments:\n");
            for (const auto& i : _cmdLineArgs)
            {
                _print("    " + i->getName());
                _print("        " + i->getHelp());
                _printNewline();
            }
            _print("Options:\n");
            for (const auto& i : _cmdLineOptions)
            {
                bool first = true;
                for (const auto& j : i->getHelpText())
                {
                    if (first)
                    {
                        first = false;
                        _print("    " + j);
                    }
                    else
                    {
                        _print("        " + j);
                    }
                }
                _printNewline();
            }
        }
    }
}
