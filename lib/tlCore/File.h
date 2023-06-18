// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <string>

namespace tl
{
    //! File system.
    namespace file
    {
        // Does a file exist?
        bool exists(const std::string&);

        // Create a directory.
        bool mkdir(const std::string&);

        // Remove a directory.
        bool rmdir(const std::string&);
        
        // Remove a file.
        bool rm(const std::string&);

        // Get the current working directory.
        std::string getCWD();

        // Get the temporary directory.
        std::string getTemp();

        // Create a temporary directory.
        std::string createTempDir();
    }
}
