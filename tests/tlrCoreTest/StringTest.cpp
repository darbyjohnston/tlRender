// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/StringTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/String.h>

#include <iostream>
#include <sstream>

using namespace tlr::string;

namespace tlr
{
    namespace CoreTest
    {
        StringTest::StringTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::StringTest", context)
        {}

        std::shared_ptr<StringTest> StringTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<StringTest>(new StringTest(context));
        }

        void StringTest::run()
        {
            _split();
            _case();
            _util();
            _convert();
            _escape();
        }
        
        void StringTest::_split()
        {
            {
                const auto pieces = split("", '/');
                TLR_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("/", '/');
                TLR_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("a", '/');
                TLR_ASSERT(1 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
            }
            {
                const auto pieces = split("/a", '/');
                TLR_ASSERT(1 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
            }
            {
                const auto pieces = split("a/", '/');
                TLR_ASSERT(1 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
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
                const auto pieces = split("", { '/', '|' });
                TLR_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("|", { '/', '|' });
                TLR_ASSERT(0 == pieces.size());
            }
            {
                const auto pieces = split("a", { '/', '|' });
                TLR_ASSERT(1 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
            }
            {
                const auto pieces = split("a/b|c||", { '/', '|' });
                TLR_ASSERT(3 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
                TLR_ASSERT("b" == pieces[1]);
                TLR_ASSERT("c" == pieces[2]);
            }
            {
                const auto pieces = split("a/b|c||", { '/', '|' }, true);
                TLR_ASSERT(4 == pieces.size());
                TLR_ASSERT("a" == pieces[0]);
                TLR_ASSERT("b" == pieces[1]);
                TLR_ASSERT("c" == pieces[2]);
                TLR_ASSERT(pieces[3].empty());
            }
            {
                TLR_ASSERT("a/b/c" == join({ "a", "b", "c" }, "/"));
            }
        }
 
        void StringTest::_case()
        {
            {
                TLR_ASSERT("ABC" == toUpper("abc"));
                TLR_ASSERT("abc" == toLower("ABC"));
            }
            {
                TLR_ASSERT(compareNoCase("abc", "ABC"));
            }
        }
 
        void StringTest::_util()
        {
            {
                std::string s = "abc";
                removeTrailingNewlines(s);
                TLR_ASSERT("abc" == s);
                s = "abc\n";
                removeTrailingNewlines(s);
                TLR_ASSERT("abc" == s);
                s = "abc\r";
                removeTrailingNewlines(s);
                TLR_ASSERT("abc" == s);
                s = "abc\n\r";
                removeTrailingNewlines(s);
                TLR_ASSERT("abc" == s);
            }
        }
 
        void StringTest::_convert()
        {
            {
                int value = 0;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLR_ASSERT(1234 == value);
            }
            {
                int value = 0;
                char buf[] = "+1234";
                fromString(buf, 5, value);
                TLR_ASSERT(1234 == value);
            }
            {
                int value = 0;
                char buf[] = "-1234";
                fromString(buf, 5, value);
                TLR_ASSERT(-1234 == value);
            }
            {
                int64_t value = 0;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLR_ASSERT(1234 == value);
            }
            {
                int64_t value = 0;
                char buf[] = "+1234";
                fromString(buf, 5, value);
                TLR_ASSERT(1234 == value);
            }
            {
                int64_t value = 0;
                char buf[] = "-1234";
                fromString(buf, 5, value);
                TLR_ASSERT(-1234 == value);
            }
            {
                size_t value = 0;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLR_ASSERT(1234 == value);
            }
            {
                float value = 0.F;
                char buf[] = "1234";
                fromString(buf, 4, value);
                TLR_ASSERT(1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "+1234.0";
                fromString(buf, 7, value);
                TLR_ASSERT(1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "-1234.0";
                fromString(buf, 7, value);
                TLR_ASSERT(-1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "1234e0";
                fromString(buf, 6, value);
                TLR_ASSERT(1234.F == value);
            }
            {
                float value = 0.F;
                char buf[] = "1234e1";
                fromString(buf, 6, value);
                TLR_ASSERT(12340.F == value);
            }
            {
                TLR_ASSERT("abc" == fromWide(toWide("abc")));
            }
        }
 
        void StringTest::_escape()
        {
            {
                TLR_ASSERT("\\\\" == escape("\\"));
                TLR_ASSERT("\\" == unescape("\\\\"));
            }
        }
   }
}
