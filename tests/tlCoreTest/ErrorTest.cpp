// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ErrorTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>

using namespace tl::core;

namespace tl
{
    namespace tests
    {
        namespace core_test
        {
            ErrorTest::ErrorTest(const std::shared_ptr<Context>& context) :
                ITest("core_test::ErrorTest", context)
            {}

            std::shared_ptr<ErrorTest> ErrorTest::create(const std::shared_ptr<Context>& context)
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
}
