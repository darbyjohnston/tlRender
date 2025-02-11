// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ColorOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/Player.h>

#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <dtk/core/CmdLine.h>

#include <filesystem>

namespace tl
{
    namespace play
    {
        //! Application options.
        struct Options
        {
            std::string fileName;
            std::string audioFileName;
            std::string compareFileName;
            timeline::CompareOptions compareOptions;
            double speed = 0.0;
            timeline::Playback playback = timeline::Playback::Stop;
            timeline::Loop loop = timeline::Loop::Loop;
            OTIO_NS::RationalTime seek = time::invalidTime;
            OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;

#if defined(TLRENDER_USD)
            int usdRenderWidth = 1920;
            float usdComplexity = 1.F;
            usd::DrawMode usdDrawMode = usd::DrawMode::ShadedSmooth;
            bool usdEnableLighting = true;
            bool usdSRGB = true;
            size_t usdStageCache = 10;
            size_t usdDiskCache = 0;
#endif // TLRENDER_USD

            std::string logFile;
            bool resetSettings = false;
            std::string settings;
        };

        //! Get the application command line arguments.
        std::vector<std::shared_ptr<dtk::ICmdLineArg> > getCmdLineArgs(Options&);

        //! Get the application command line options.
        std::vector<std::shared_ptr<dtk::ICmdLineOption> > getCmdLineOptions(
            Options&,
            const std::filesystem::path& logFilePath,
            const std::filesystem::path& settingsFilePath);
    }
}
