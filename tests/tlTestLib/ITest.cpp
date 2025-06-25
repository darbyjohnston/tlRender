// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTestLib/ITest.h>

#include <iostream>

namespace tl
{
    namespace tests
    {
        ITest::ITest(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::string& name) :
            _context(context),
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
