// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ErrorTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Error.h>

using namespace tlr::core;

namespace tlr
{
    namespace CoreTest
    {
        ErrorTest::ErrorTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::ErrorTest", context)
        {}

        std::shared_ptr<ErrorTest> ErrorTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<ErrorTest>(new ErrorTest(context));
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
