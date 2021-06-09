// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <string>
#include <vector>

namespace tlr
{
    //! File system.
    namespace file
    {
        //! Is a path absolute?
        bool isAbsolute(const std::string& path);

        //! Convert a path to use UNIX style path delimeters ('/').
        std::string normalize(const std::string& path);

        //! Split a file name into pieces: path, base name, number, and extension.
        void split(
            const std::string& fileName,
            std::string* path,
            std::string* baseName = nullptr,
            std::string* number = nullptr,
            std::string* extension = nullptr);

        // Create a temporary directory.
        std::string createTempDir();
    }
}