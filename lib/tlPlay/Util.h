// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <string>

namespace tl
{
    namespace play
    {
        //! Get the path to the application directory. The directory is
        //! automatically created if it does not exist.
        std::string appDirPath(const std::string& appName);

        //! Get the log file name.
        std::string logFileName(const std::string& appDirPath);

        //! Get the settings file name.
        std::string settingsName(const std::string& appDirPath);
    }
}
