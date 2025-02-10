// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <filesystem>
#include <string>

namespace tl
{
    //! Common functionality for the "tlplay" and "tlplay-qt" applications.
    namespace play
    {
        //! Get the path to the application documents directory. The directory
        //! is automatically created if it does not exist.
        std::filesystem::path appDocsPath();

        //! Get the log file path.
        std::filesystem::path logFileName(
            const std::string& appName,
            const std::filesystem::path& appDocsPath);

        //! Get the settings file path.
        std::filesystem::path settingsName(
            const std::string& appName,
            const std::filesystem::path& appDocsPath);
    }
}
