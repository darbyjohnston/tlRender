// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/Util.h>

#include <tlCore/Path.h>

#include <dtk/core/File.h>
#include <dtk/core/Format.h>

#include <filesystem>

namespace tl
{
    namespace play
    {
        std::filesystem::path appDocsPath()
        {
            const std::filesystem::path documentsPath = dtk::getUserPath(
                dtk::UserPath::Documents);
            if (!std::filesystem::exists(documentsPath))
            {
                std::filesystem::create_directory(documentsPath);
            }
            const std::filesystem::path out = documentsPath / "tlRender";
            if (!std::filesystem::exists(out))
            {
                std::filesystem::create_directory(out);
            }
            return out;
        }

        std::filesystem::path logFileName(
            const std::string& appName,
            const std::filesystem::path& appDocsPath)
        {
            return appDocsPath / dtk::Format("{0}.{1}.log").
                arg(appName).
                arg(TLRENDER_VERSION).
                str();
        }

        std::filesystem::path settingsName(
            const std::string& appName,
            const std::filesystem::path& appDocsPath)
        {
            return appDocsPath / dtk::Format("{0}.{1}.json").
                arg(appName).
                arg(TLRENDER_VERSION).
                str();
        }
    }
}
