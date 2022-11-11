// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlApp/IApp.h>

#include <tlIO/IOSystem.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <iostream>

namespace tl
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
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        void IApp::_init(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<ICmdLineArg> >& args,
            const std::vector<std::shared_ptr<ICmdLineOption> >& options)
        {
            TLRENDER_P();

            _context = context;

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
                _options.sequenceDefaultSpeed,
                { "-sequenceDefaultSpeed" },
                "Default speed for image sequences.",
                string::Format("{0}").arg(_options.sequenceDefaultSpeed)));
            p.cmdLineOptions.push_back(CmdLineValueOption<int>::create(
                _options.sequenceThreadCount,
                { "-sequenceThreadCount" },
                "Number of threads for image sequence I/O.",
                string::Format("{0}").arg(_options.sequenceThreadCount)));
#if defined(TLRENDER_EXR)
            p.cmdLineOptions.push_back(CmdLineValueOption<exr::Compression>::create(
                _options.exrCompression,
                { "-exrCompression" },
                "OpenEXR output compression.",
                string::Format("{0}").arg(_options.exrCompression),
                string::join(exr::getCompressionLabels(), ", ")));
            p.cmdLineOptions.push_back(CmdLineValueOption<float>::create(
                _options.exrDWACompressionLevel,
                { "-exrDWACompressionLevel" },
                "OpenEXR DWA compression level.",
                string::Format("{0}").arg(_options.exrDWACompressionLevel)));
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
            p.cmdLineOptions.push_back(CmdLineValueOption<int>::create(
                _options.ffmpegThreadCount,
                { "-ffmpegThreadCount" },
                "Number of threads for FFmpeg I/O.",
                string::Format("{0}").arg(_options.ffmpegThreadCount)));
            p.cmdLineOptions.push_back(app::CmdLineValueOption<std::string>::create(
                _options.ffmpegWriteProfile,
                { "-ffmpegProfile", "-ffp" },
                "FFmpeg output profile.",
                std::string(),
                string::join(ffmpeg::getProfileLabels(), ", ")));
#endif // TLRENDER_FFMPEG
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
                p.logObserver = observer::ListObserver<log::Item>::create(
                    context->getSystem<log::System>()->observeLog(),
                    [this](const std::vector<log::Item>& value)
                    {
                        for (const auto& i : value)
                        {
                            _print("[LOG] " + toString(i));
                        }
                    },
                    observer::CallbackAction::Suppress);
            }

            // Set I/O options.
            io::Options ioOptions;
            {
                std::stringstream ss;
                ss << _options.sequenceDefaultSpeed;
                ioOptions["SequenceIO/DefaultSpeed"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.sequenceThreadCount;
                ioOptions["SequenceIO/ThreadCount"] = ss.str();
            }
#if defined(TLRENDER_EXR)
            {
                std::stringstream ss;
                ss << _options.exrCompression;
                ioOptions["exr/Compression"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << _options.exrDWACompressionLevel;
                ioOptions["exr/DWACompressionLevel"] = ss.str();
            }
#endif // TLRENDER_EXR
#if defined(TLRENDER_FFMPEG)
            if (!_options.ffmpegWriteProfile.empty())
            {
                ioOptions["ffmpeg/WriteProfile"] = _options.ffmpegWriteProfile;
            }
            {
                std::stringstream ss;
                ss << _options.ffmpegThreadCount;
                ioOptions["ffmpeg/ThreadCount"] = ss.str();
            }
#endif // TLRENDER_FFMPEG
            context->getSystem<io::System>()->setOptions(ioOptions);
        }
        
        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {}

        const std::shared_ptr<system::Context>& IApp::getContext() const
        {
            return _context;
        }

        int IApp::getExit() const
        {
            return _exit;
        }

        void IApp::_log(const std::string& value, log::Type type)
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
            TLRENDER_P();
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
            TLRENDER_P();
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
