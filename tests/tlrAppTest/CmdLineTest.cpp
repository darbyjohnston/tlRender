// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrAppTest/CmdLineTest.h>

namespace tlr
{
    namespace AppTest
    {
        CmdLineTest::CmdLineTest() :
            ITest("AppTest::CmdLineTest")
        {}

        std::shared_ptr<CmdLineTest> CmdLineTest::create()
        {
            return std::shared_ptr<CmdLineTest>(new CmdLineTest);
        }

        void CmdLineTest::run()
        {
        }
    }
}
