// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppTest/CmdLineTest.h>

namespace tl
{
    namespace tests
    {
        namespace AppTest
        {
            CmdLineTest::CmdLineTest(const std::shared_ptr<core::Context>& context) :
                ITest("AppTest::CmdLineTest", context)
            {}

            std::shared_ptr<CmdLineTest> CmdLineTest::create(const std::shared_ptr<core::Context>& context)
            {
                return std::shared_ptr<CmdLineTest>(new CmdLineTest(context));
            }

            void CmdLineTest::run()
            {
            }
        }
    }
}
