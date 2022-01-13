// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <string>
#include <vector>

namespace tlr
{
    //! Operating system functionality.
    namespace os
    {
        //! \name Information
        ///@{

        //! Operating system information.
        struct SystemInfo
        {
            std::string name;
            size_t cores     = 0;
            size_t ram       = 0;
            size_t ramGB     = 0;
        };

        //! Get operating system information.
        SystemInfo getSystemInfo();

        ///@}

        //! \name Environment Variables
        ///@{

        //! Environment variable list separators.
        enum class EnvListSeparator
        {
            Unix,
            Windows
        };

        //! Get an environment variable list separator.
        char getEnvListSeparator(EnvListSeparator);

        //! Get the current environment variable list separator.
        char getEnvListSeparator();

        //! Get an environment variable.
        //! 
        //! Throws:
        //! - std::exception
        bool getEnv(const std::string& name, std::string&);

        //! Get an environment variable and convert it to an integer. If the
        //! variable is empty then zero is returned.
        //!
        //! Throws:
        //! - std::exception
        bool getIntEnv(const std::string& name, int& value);

        //! Get an environment variable and convert it to a list of strings.
        //!
        //! Throws:
        //! - std::exception
        bool getStringListEnv(const std::string& name, std::vector<std::string>&);

        //! Set an environment variable.
        //! 
        //! Throws:
        //! - std::exception
        bool setEnv(const std::string& name, const std::string&);
        
        //! Delete an environment variable.
        //! 
        //! Throws:
        //! - std::exception
        bool delEnv(const std::string& name);
            
        ///@}
    }
}
