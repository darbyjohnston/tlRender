// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/StringFormatTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/StringFormat.h>

using namespace tlr::string;

namespace tlr
{
    namespace CoreTest
    {
        StringFormatTest::StringFormatTest() :
            ITest("CoreTest::StringFormatTest")
        {}

        std::shared_ptr<StringFormatTest> StringFormatTest::create()
        {
            return std::shared_ptr<StringFormatTest>(new StringFormatTest);
        }

        void StringFormatTest::run()
        {
            {
                const std::string s = Format("");
                TLR_ASSERT(s.empty());
            }
            {
                const std::string s = Format("abc");
                TLR_ASSERT("abc" == s);
            }
            {
                const std::string s = Format("{0}{1}{2}").arg("a").arg("b").arg("c");
                TLR_ASSERT("abc" == s);
            }
            {
                const std::string s = Format("{0}{1}{2}").arg(1).arg(2).arg(3);
                TLR_ASSERT("123" == s);
            }
            {
                const std::string s = Format("{0}").arg(1.0F, 2);
                TLR_ASSERT("1.00" == s);
            }
            {
                const std::string s = Format("{0}").arg(1.0, 2);
                TLR_ASSERT("1.00" == s);
            }
            {
                const auto f = Format("").arg(1);
                TLR_ASSERT(f.hasError());
                std::stringstream ss;
                ss << "String format error: " << f.getError();
                _print(ss.str());
            }
            {
                const auto f = Format("{0}{0}").arg(0).arg(1);
                TLR_ASSERT(f.hasError());
                std::stringstream ss;
                ss << "String format error: " << f.getError();
                _print(ss.str());
            }
        }
    }
}
