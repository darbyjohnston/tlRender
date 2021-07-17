// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrApp/IApp.h>

#include <tlrCore/AVIOSystem.h>
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
        struct IApp::Private
        {
            std::vector<std::string> cmdLine;
            std::string cmdLineName;
            std::string cmdLineSummary;
            std::vector<std::shared_ptr<ICmdLineArg> > cmdLineArgs;
            std::vector<std::shared_ptr<ICmdLineOption> > cmdLineOptions;
            std::shared_ptr<observer::ValueObserver<core::LogItem> > logObserver;
        };

        void IApp::_init(
            int argc,
            char* argv[],
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<ICmdLineArg> >& args,
            const std::vector<std::shared_ptr<ICmdLineOption> >& options)
        {
            TLR_PRIVATE_P();

            // Create the context.
            _context = core::Context::create();

            // Parse the command line.
            for (int i = 1; i < argc; ++i)
            {
                p.cmdLine.push_back(argv[i]);
            }
            p.cmdLineName = cmdLineName;
            p.cmdLineSummary = cmdLineSummary;
            p.cmdLineArgs = args;
            p.cmdLineOptions = options;
            p.cmdLineOptions.push_back(CmdLineValueOption<float>::create(
                _options.seqDefaultSpeed,
                { "-seqDefaultSpeed" },
                "Default speed for image sequences.",
                string::Format("{0}").arg(_options.seqDefaultSpeed)));
            p.cmdLineOptions.push_back(CmdLineValueOption<int>::create(
                _options.seqThreadCount,
                { "-seqThreadCount" },
                "Number of threads for image sequence I/O.",
                string::Format("{0}").arg(_options.seqThreadCount)));
#if defined(FFmpeg_FOUND)
            p.cmdLineOptions.push_back(CmdLineValueOption<int>::create(
                _options.ffThreadCount,
                { "-ffThreadCount" },
                "Number of threads for FFmpeg I/O.",
                string::Format("{0}").arg(_options.ffThreadCount)));
            p.cmdLineOptions.push_back(app::CmdLineValueOption<std::string>::create(
                _options.ffWriteProfile,
                { "-ffProfile", "-ffp" },
                "FFmpeg output profile.",
                std::string(),
                string::join(ffmpeg::getProfileLabels(), ", ")));
#endif
            p.cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.log,
                { "-log" },
                "Print the log to the console."));
            p.cmdLineOptions.push_back(CmdLineFlagOption::create(
                _options.help,
                { "-help", "-h", "--help", "--h" },
                "Show this message."));
            _exit = _parseCmdLine();

            // Setup the log.
            if (_options.log)
            {
                for (const auto& i : _context->getLogInit())
                {
                    _print("[LOG] " + core::toString(i));
                }
                p.logObserver = observer::ValueObserver<core::LogItem>::create(
                    _context->getSystem<core::LogSystem>()->observeLog(),
                    [this](const core::LogItem& value)
                    {
                        _print("[LOG] " + core::toString(value));
                    });
            }

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
            _context->getSystem<avio::System>()->setOptions(avioOptions);
        }
        
        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {}

        int IApp::getExit() const
        {
            return _exit;
        }

        void IApp::_log(const std::string& value, core::LogType type)
        {
            _context->log(_p->cmdLineName, value, type);
        }

        void IApp::_print(const std::string& value)
        {
            std::cout << value << std::endl;
        }

        void IApp::_printNewline()
        {
            std::cout << std::endl;
        }

        void IApp::_printError(const std::string& value)
        {
            std::cerr << "ERROR: " << value << std::endl;
        }

        int IApp::_parseCmdLine()
        {
            TLR_PRIVATE_P();
            for (const auto& i : p.cmdLineOptions)
            {
                try
                {
                    i->parse(p.cmdLine);
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
            for (const auto& i : p.cmdLineArgs)
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
            if (p.cmdLine.size() < requiredArgs ||
                p.cmdLine.size() > requiredArgs + optionalArgs ||
                _options.help)
            {
                _printCmdLineHelp();
                return 1;
            }
            for (const auto& i : p.cmdLineArgs)
            {
                try
                {
                    if (!(p.cmdLine.empty() && i->isOptional()))
                    {
                        i->parse(p.cmdLine);
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
            TLR_PRIVATE_P();
            _print("\n" + p.cmdLineName + "\n");
            _print("    " + p.cmdLineSummary + "\n");
            _print("Usage:\n");
            {
                std::stringstream ss;
                ss << "    " + p.cmdLineName;
                if (p.cmdLineArgs.size())
                {
                    std::vector<std::string> args;
                    for (const auto& i : p.cmdLineArgs)
                    {
                        const bool optional = i->isOptional();
                        args.push_back(
                            (optional ? "[" : "(") +
                            string::toLower(i->getName()) +
                            (optional ? "]" : ")"));
                    }
                    ss << " " << string::join(args, " ");
                }
                if (p.cmdLineOptions.size())
                {
                    ss << " [option],...";
                }
                _print(ss.str());
                _printNewline();
            }
            _print("Arguments:\n");
            for (const auto& i : p.cmdLineArgs)
            {
                _print("    " + i->getName());
                _print("        " + i->getHelp());
                _printNewline();
            }
            _print("Options:\n");
            for (const auto& i : p.cmdLineOptions)
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
