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

        //! Get operating system information.
        std::string getInfo();

        //! Get the total amount of system RAM.
        size_t getRAMSize();

        //! Get the total amount of system RAM in gigabytes.
        size_t getRAMSizeGB();

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
