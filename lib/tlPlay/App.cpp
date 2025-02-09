// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/App.h>

#include <dtk/core/Format.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace play
    {
        std::vector<std::shared_ptr<dtk::ICmdLineArg> > getCmdLineArgs(Options& options)
        {
            return
            {
                dtk::CmdLineValueArg<std::string>::create(
                    options.fileName,
                    "input",
                    "Timeline, movie, image sequence, or folder.",
                    true)
            };
        }

        std::vector<std::shared_ptr<dtk::ICmdLineOption> > getCmdLineOptions(
            Options& options,
            const std::string& logFileName,
            const std::string& settingsFileName)
        {
            return
            {
                dtk::CmdLineValueOption<std::string>::create(
                    options.audioFileName,
                    { "-audio", "-a" },
                    "Audio file name."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.compareFileName,
                    { "-b" },
                    "A/B comparison \"B\" file name."),
                dtk::CmdLineValueOption<timeline::CompareMode>::create(
                    options.compareOptions.mode,
                    { "-compare", "-c" },
                    "A/B comparison mode.",
                    dtk::Format("{0}").arg(options.compareOptions.mode),
                    dtk::join(timeline::getCompareModeLabels(), ", ")),
                dtk::CmdLineValueOption<dtk::V2F>::create(
                    options.compareOptions.wipeCenter,
                    { "-wipeCenter", "-wc" },
                    "A/B comparison wipe center.",
                    dtk::Format("{0}").arg(options.compareOptions.wipeCenter)),
                dtk::CmdLineValueOption<float>::create(
                    options.compareOptions.wipeRotation,
                    { "-wipeRotation", "-wr" },
                    "A/B comparison wipe rotation.",
                    dtk::Format("{0}").arg(options.compareOptions.wipeRotation)),
                dtk::CmdLineValueOption<double>::create(
                    options.speed,
                    { "-speed" },
                    "Playback speed."),
                dtk::CmdLineValueOption<timeline::Playback>::create(
                    options.playback,
                    { "-playback", "-p" },
                    "Playback mode.",
                    dtk::Format("{0}").arg(options.playback),
                    dtk::join(timeline::getPlaybackLabels(), ", ")),
                dtk::CmdLineValueOption<timeline::Loop>::create(
                    options.loop,
                    { "-loop", "-lp" },
                    "Playback loop mode.",
                    dtk::Format("{0}").arg(options.loop),
                    dtk::join(timeline::getLoopLabels(), ", ")),
                dtk::CmdLineValueOption<OTIO_NS::RationalTime>::create(
                    options.seek,
                    { "-seek" },
                    "Seek to the given time."),
                dtk::CmdLineValueOption<OTIO_NS::TimeRange>::create(
                    options.inOutRange,
                    { "-inOutRange" },
                    "Set the in/out points range."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.fileName,
                    { "-ocio" },
                    "OpenColorIO configuration file name (e.g., config.ocio)."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.input,
                    { "-ocioInput" },
                    "OpenColorIO input name."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.display,
                    { "-ocioDisplay" },
                    "OpenColorIO display name."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.view,
                    { "-ocioView" },
                    "OpenColorIO view name."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.ocioOptions.look,
                    { "-ocioLook" },
                    "OpenColorIO look name."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.lutOptions.fileName,
                    { "-lut" },
                    "LUT file name."),
                dtk::CmdLineValueOption<timeline::LUTOrder>::create(
                    options.lutOptions.order,
                    { "-lutOrder" },
                    "LUT operation order.",
                    dtk::Format("{0}").arg(options.lutOptions.order),
                    dtk::join(timeline::getLUTOrderLabels(), ", ")),
#if defined(TLRENDER_USD)
                dtk::CmdLineValueOption<int>::create(
                    options.usdRenderWidth,
                    { "-usdRenderWidth" },
                    "USD render width.",
                    dtk::Format("{0}").arg(options.usdRenderWidth)),
                dtk::CmdLineValueOption<float>::create(
                    options.usdComplexity,
                    { "-usdComplexity" },
                    "USD render complexity setting.",
                    dtk::Format("{0}").arg(options.usdComplexity)),
                dtk::CmdLineValueOption<usd::DrawMode>::create(
                    options.usdDrawMode,
                    { "-usdDrawMode" },
                    "USD draw mode.",
                    dtk::Format("{0}").arg(options.usdDrawMode),
                    dtk::join(usd::getDrawModeLabels(), ", ")),
                dtk::CmdLineValueOption<bool>::create(
                    options.usdEnableLighting,
                    { "-usdEnableLighting" },
                    "USD enable lighting.",
                    dtk::Format("{0}").arg(options.usdEnableLighting)),
                dtk::CmdLineValueOption<bool>::create(
                    options.usdSRGB,
                    { "-usdSRGB" },
                    "USD enable sRGB color space.",
                    dtk::Format("{0}").arg(options.usdSRGB)),
                dtk::CmdLineValueOption<size_t>::create(
                    options.usdStageCache,
                    { "-usdStageCache" },
                    "USD stage cache size.",
                    dtk::Format("{0}").arg(options.usdStageCache)),
                dtk::CmdLineValueOption<size_t>::create(
                    options.usdDiskCache,
                    { "-usdDiskCache" },
                    "USD disk cache size in gigabytes. A size of zero disables the disk cache.",
                    dtk::Format("{0}").arg(options.usdDiskCache)),
#endif // TLRENDER_USD
                dtk::CmdLineValueOption<std::string>::create(
                    options.logFileName,
                    { "-logFile" },
                    "Log file name.",
                    dtk::Format("{0}").arg(logFileName)),
                dtk::CmdLineFlagOption::create(
                    options.resetSettings,
                    { "-resetSettings" },
                    "Reset settings to defaults."),
                dtk::CmdLineValueOption<std::string>::create(
                    options.settingsFileName,
                    { "-settings" },
                    "Settings file name.",
                    dtk::Format("{0}").arg(settingsFileName)),
            };
        }
    }
}
