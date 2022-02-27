// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/StringTest.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>

#include <iostream>
#include <sstream>

using namespace tl::core;
using namespace tl::core::string;

namespace tl
{
    namespace tests
    {
        namespace core_test
        {
            StringTest::StringTest(const std::shared_ptr<core::Context>& context) :
                ITest("core_test::StringTest", context)
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
                    TLRENDER_ASSERT(0 == pieces.size());
                }
                {
                    const auto pieces = split("/", '/');
                    TLRENDER_ASSERT(0 == pieces.size());
                }
                {
                    const auto pieces = split("a", '/');
                    TLRENDER_ASSERT(1 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                }
                {
                    const auto pieces = split("/a", '/');
                    TLRENDER_ASSERT(1 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                }
                {
                    const auto pieces = split("a/", '/');
                    TLRENDER_ASSERT(1 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                }
                {
                    const auto pieces = split("a/b/c//", '/');
                    TLRENDER_ASSERT(3 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                    TLRENDER_ASSERT("b" == pieces[1]);
                    TLRENDER_ASSERT("c" == pieces[2]);
                }
                {
                    const auto pieces = split("a/b/c//", '/', true);
                    TLRENDER_ASSERT(4 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                    TLRENDER_ASSERT("b" == pieces[1]);
                    TLRENDER_ASSERT("c" == pieces[2]);
                    TLRENDER_ASSERT(pieces[3].empty());
                }
                {
                    const auto pieces = split("", { '/', '|' });
                    TLRENDER_ASSERT(0 == pieces.size());
                }
                {
                    const auto pieces = split("|", { '/', '|' });
                    TLRENDER_ASSERT(0 == pieces.size());
                }
                {
                    const auto pieces = split("a", { '/', '|' });
                    TLRENDER_ASSERT(1 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                }
                {
                    const auto pieces = split("a/b|c||", { '/', '|' });
                    TLRENDER_ASSERT(3 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                    TLRENDER_ASSERT("b" == pieces[1]);
                    TLRENDER_ASSERT("c" == pieces[2]);
                }
                {
                    const auto pieces = split("a/b|c||", { '/', '|' }, true);
                    TLRENDER_ASSERT(4 == pieces.size());
                    TLRENDER_ASSERT("a" == pieces[0]);
                    TLRENDER_ASSERT("b" == pieces[1]);
                    TLRENDER_ASSERT("c" == pieces[2]);
                    TLRENDER_ASSERT(pieces[3].empty());
                }
                {
                    TLRENDER_ASSERT("a/b/c" == join({ "a", "b", "c" }, "/"));
                }
            }

            void StringTest::_case()
            {
                {
                    TLRENDER_ASSERT("ABC" == toUpper("abc"));
                    TLRENDER_ASSERT("abc" == toLower("ABC"));
                }
                {
                    TLRENDER_ASSERT(compareNoCase("abc", "ABC"));
                }
            }

            void StringTest::_util()
            {
                {
                    std::string s = "abc";
                    removeTrailingNewlines(s);
                    TLRENDER_ASSERT("abc" == s);
                    s = "abc\n";
                    removeTrailingNewlines(s);
                    TLRENDER_ASSERT("abc" == s);
                    s = "abc\r";
                    removeTrailingNewlines(s);
                    TLRENDER_ASSERT("abc" == s);
                    s = "abc\n\r";
                    removeTrailingNewlines(s);
                    TLRENDER_ASSERT("abc" == s);
                }
            }

            void StringTest::_convert()
            {
                {
                    int value = 0;
                    char buf[] = "1234";
                    fromString(buf, 4, value);
                    TLRENDER_ASSERT(1234 == value);
                }
                {
                    int value = 0;
                    char buf[] = "+1234";
                    fromString(buf, 5, value);
                    TLRENDER_ASSERT(1234 == value);
                }
                {
                    int value = 0;
                    char buf[] = "-1234";
                    fromString(buf, 5, value);
                    TLRENDER_ASSERT(-1234 == value);
                }
                {
                    int64_t value = 0;
                    char buf[] = "1234";
                    fromString(buf, 4, value);
                    TLRENDER_ASSERT(1234 == value);
                }
                {
                    int64_t value = 0;
                    char buf[] = "+1234";
                    fromString(buf, 5, value);
                    TLRENDER_ASSERT(1234 == value);
                }
                {
                    int64_t value = 0;
                    char buf[] = "-1234";
                    fromString(buf, 5, value);
                    TLRENDER_ASSERT(-1234 == value);
                }
                {
                    size_t value = 0;
                    char buf[] = "1234";
                    fromString(buf, 4, value);
                    TLRENDER_ASSERT(1234 == value);
                }
                {
                    float value = 0.F;
                    char buf[] = "1234";
                    fromString(buf, 4, value);
                    TLRENDER_ASSERT(1234.F == value);
                }
                {
                    float value = 0.F;
                    char buf[] = "+1234.0";
                    fromString(buf, 7, value);
                    TLRENDER_ASSERT(1234.F == value);
                }
                {
                    float value = 0.F;
                    char buf[] = "-1234.0";
                    fromString(buf, 7, value);
                    TLRENDER_ASSERT(-1234.F == value);
                }
                {
                    float value = 0.F;
                    char buf[] = "1234e0";
                    fromString(buf, 6, value);
                    TLRENDER_ASSERT(1234.F == value);
                }
                {
                    float value = 0.F;
                    char buf[] = "1234e1";
                    fromString(buf, 6, value);
                    TLRENDER_ASSERT(12340.F == value);
                }
                {
                    TLRENDER_ASSERT("abc" == fromWide(toWide("abc")));
                }
            }

            void StringTest::_escape()
            {
                {
                    TLRENDER_ASSERT("\\\\" == escape("\\"));
                    TLRENDER_ASSERT("\\" == unescape("\\\\"));
                }
            }
        }
    }
}
