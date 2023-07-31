// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlay/Util.h>

#include <tlCore/File.h>
#include <tlCore/Path.h>

namespace tl
{
    namespace play
    {
        std::string appDirPath(const std::string& appName)
        {
            const std::string documentsPath = file::getUserPath(file::UserPath::Documents);
            if (!file::exists(documentsPath))
            {
                file::mkdir(documentsPath);
            }
            const std::string appDirPath = file::Path(documentsPath, appName).get();
            if (!file::exists(appDirPath))
            {
                file::mkdir(appDirPath);
            }
            return appDirPath;
        }

        std::string logFileName(const std::string& appDirPath)
        {
            return file::Path(appDirPath, "log.txt").get();
        }

        std::string settingsName(const std::string& appDirPath)
        {
            return file::Path(appDirPath, "settings.json").get();
        }
    }
}
