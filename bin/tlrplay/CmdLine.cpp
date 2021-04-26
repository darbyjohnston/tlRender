#include "App.h"

#include <tlrApp/CmdLine.h>

#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

#include <sstream>

namespace tlr
{
    int App::_parseCmdLine()
    {
        std::vector<std::unique_ptr<app::IOption> > options;
        options.emplace_back(new app::ValueOption<float>(
            _options.windowScale,
            { "-windowScale", "-ws" },
            string::Format("Set the window size scale factor. Default: {0}").
                arg(_options.windowScale),
            "(value)"));
        options.emplace_back(new app::FlagOption(
            _options.fullScreen,
            { "-fullScreen", "-fs" },
            "Enable full screen mode."));
        options.emplace_back(new app::ValueOption<bool>(
            _options.hud,
            { "-hud" },
            string::Format("Enable the HUD (heads up display). Default: {0}").
            arg(_options.hud),
            "(value)"));
        options.emplace_back(new app::ValueOption<bool>(
            _options.startPlayback,
            { "-startPlayback", "-sp" },
            string::Format("Automatically start playback. Default: {0}").
            arg(_options.startPlayback),
            "(value)"));
        options.emplace_back(new app::ValueOption<bool>(
            _options.loopPlayback,
            { "-loopPlayback", "-lp" },
            string::Format("Loop playback. Default: {0}").
            arg(_options.loopPlayback),
            "(value)"));
        options.emplace_back(new app::ValueOption<size_t>(
            _options.ioVideoQueueSize,
            { "-ioVideoQueueSize", "-vqs" },
            string::Format("Set the video queue size. Default: {0}").
            arg(_options.ioVideoQueueSize),
            "(value)"));
        options.emplace_back(new app::FlagOption(
            _options.verbose,
            {"-verbose", "-v" },
            "Enable verbose mode."));
        options.emplace_back(new app::FlagOption(
            _options.help,
            { "-help", "-h", "--help", "--h" },
            "Show this message."));

        for (const auto& i : options)
        {
            try
            {
                i->parse(_args);
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot parse option \"" << i->getName() << "\": " << e.what();
                throw std::runtime_error(ss.str());
            }
        }

        if (_args.size() != 1 || _options.help)
        {
            _print(
                "\n"
                "Usage:\n"
                "\n"
                "    tlrplay (input) [option]...\n"
                "\n"
                "Arguments:\n"
                "\n"
                "    input - Input timeline\n"
                "\n"
                "Options:\n");
            for (const auto& option : options)
            {
                _print(
                    "    " + string::join(option->getNames(), ", ") + " " + option->getArgsHelp() + "\n"
                    "    " + option->getHelp() + "\n");
            }
            return 1;
        }

        _input = _args[0];

        return 0;
    }
}
