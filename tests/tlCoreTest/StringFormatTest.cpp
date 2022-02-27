// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/StringFormatTest.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

using namespace tl::core;
using namespace tl::core::string;

namespace tl
{
    namespace tests
    {
        namespace core_test
        {
            StringFormatTest::StringFormatTest(const std::shared_ptr<core::Context>& context) :
                ITest("core_test::StringFormatTest", context)
            {}

            std::shared_ptr<StringFormatTest> StringFormatTest::create(const std::shared_ptr<core::Context>& context)
            {
                return std::shared_ptr<StringFormatTest>(new StringFormatTest(context));
            }

            void StringFormatTest::run()
            {
                {
                    const std::string s = Format("");
                    TLRENDER_ASSERT(s.empty());
                }
                {
                    const std::string s = Format("abc");
                    TLRENDER_ASSERT("abc" == s);
                }
                {
                    const std::string s = Format("{0}{1}{2}").arg("a").arg("b").arg("c");
                    TLRENDER_ASSERT("abc" == s);
                }
                {
                    const std::string s = Format("{0}{1}{2}").arg(1).arg(2).arg(3);
                    TLRENDER_ASSERT("123" == s);
                }
                {
                    const std::string s = Format("{0}").arg(1.0F, 2);
                    TLRENDER_ASSERT("1.00" == s);
                }
                {
                    const std::string s = Format("{0}").arg(1.0, 2);
                    TLRENDER_ASSERT("1.00" == s);
                }
                {
                    const auto f = Format("").arg(1);
                    TLRENDER_ASSERT(f.hasError());
                    std::stringstream ss;
                    ss << "String format error: " << f.getError();
                    _print(ss.str());
                }
                {
                    const auto f = Format("{0}{0}").arg(0).arg(1);
                    TLRENDER_ASSERT(f.hasError());
                    std::stringstream ss;
                    ss << "String format error: " << f.getError();
                    _print(ss.str());
                }
            }
        }
    }
}
