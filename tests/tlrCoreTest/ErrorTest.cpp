// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ErrorTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Error.h>

namespace tlr
{
    namespace CoreTest
    {
        ErrorTest::ErrorTest() :
            ITest("CoreTest::ErrorTest")
        {}

        std::shared_ptr<ErrorTest> ErrorTest::create()
        {
            return std::shared_ptr<ErrorTest>(new ErrorTest);
        }

        void ErrorTest::run()
        {
            try
            {
                throw ParseError();
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }
        }
    }
}
