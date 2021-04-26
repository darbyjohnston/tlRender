// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <string>
#include <vector>

namespace tlr
{
    //! Strings
    namespace string
    {
        const size_t cBufferSize = 4096;

        // Split a string.
        std::vector<std::string> split(const std::string&, char delimeter, bool keepEmpty = false);

        // Split a string.
        std::vector<std::string> split(const std::string&, const std::vector<char>& delimeters, bool keepEmpty = false);

        //! Join a list of strings.
        std::string join(const std::vector<std::string>&, const std::string& delimeter);

        //! Convert to upper case.
        std::string toUpper(const std::string&);

        //! Convert to lower case.
        std::string toLower(const std::string&);

        //! Compare case insensitive.
        bool compareNoCase(const std::string&, const std::string&);
    }
}