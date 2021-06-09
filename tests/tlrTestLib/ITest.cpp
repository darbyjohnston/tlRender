// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrTestLib/ITest.h>

#include <iostream>

namespace tlr
{
    namespace Test
    {
        ITest::ITest(const std::string& name) :
            _name(name)
        {}

        ITest::~ITest()
        {}

        const std::string& ITest::getName() const
        {
            return _name;
        }

        void ITest::_print(const std::string& value)
        {
            std::cout << "    " << value << std::endl;
        }

        void ITest::_printError(const std::string& value)
        {
            std::cout << "    ERROR: " << value << std::endl;
        }
    }
}
