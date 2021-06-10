// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/StringTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/String.h>

#include <sstream>

using namespace tlr::string;

namespace tlr
{
    namespace CoreTest
    {
        StringTest::StringTest() :
            ITest("CoreTest::StringTest")
        {}

        std::shared_ptr<StringTest> StringTest::create()
        {
            return std::shared_ptr<StringTest>(new StringTest);
        }

        void StringTest::run()
        {
            {
                std::stringstream ss;
                ss << "C string buffer size: " << cBufferSize;
                _print(ss.str());
            }
            {
                const auto pieces = split("a/b/c", '/');
                TLR_ASSERT(3 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
                TLR_ASSERT("b" == pieces[1]);
                TLR_ASSERT("c" == pieces[2]);
            }
            {
                const auto pieces = split("a/b/c//", '/');
                TLR_ASSERT(3 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
                TLR_ASSERT("b" == pieces[1]);
                TLR_ASSERT("c" == pieces[2]);
            }
            {
                const auto pieces = split("a/b/c//", '/', true);
                TLR_ASSERT(4 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
                TLR_ASSERT("b" == pieces[1]);
                TLR_ASSERT("c" == pieces[2]);
                TLR_ASSERT(pieces[3].empty());
            }
            {
                TLR_ASSERT("a/b/c" == join({ "a", "b", "c" }, "/"));
            }
            {
                TLR_ASSERT("ABC" == toUpper("abc"));
                TLR_ASSERT("abc" == toLower("ABC"));
            }
            {
                TLR_ASSERT(compareNoCase("abc", "ABC"));
            }
        }
    }
}
