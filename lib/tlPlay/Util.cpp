// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/Util.h>

#include <tlCore/Path.h>

#include <dtk/core/Format.h>

#include <filesystem>

namespace tl
{
    namespace play
    {
        std::string appDocsPath()
        {
            const std::string documentsPath = file::getUserPath(
                file::UserPath::Documents);
            if (!std::filesystem::exists(std::filesystem::u8path(documentsPath)))
            {
                std::filesystem::create_directory(std::filesystem::u8path(documentsPath));
            }
            const std::string out = file::Path(
                documentsPath,
                "tlRender").get();
            if (!std::filesystem::exists(std::filesystem::u8path(out)))
            {
                std::filesystem::create_directory(std::filesystem::u8path(out));
            }
            return out;
        }

        std::string logFileName(
            const std::string& appName,
            const std::string& appDirPath)
        {
            const std::string fileName = dtk::Format("{0}.{1}.log").
                arg(appName).
                arg(TLRENDER_VERSION);
            return file::Path(appDirPath, fileName).get();
        }

        std::string settingsName(
            const std::string& appName,
            const std::string& appDirPath)
        {
            const std::string fileName = dtk::Format("{0}.{1}.json").
                arg(appName).
                arg(TLRENDER_VERSION);
            return file::Path(appDirPath, fileName).get();
        }
    }
}
