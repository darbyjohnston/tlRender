// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/ColorConfigOptionsTest.h>

#include <tlTimeline/ColorConfigOptions.h>

#include <tlCore/Assert.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        ColorConfigOptionsTest::ColorConfigOptionsTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::ColorConfigOptionsTest", context)
        {}

        std::shared_ptr<ColorConfigOptionsTest> ColorConfigOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ColorConfigOptionsTest>(new ColorConfigOptionsTest(context));
        }

        void ColorConfigOptionsTest::run()
        {
            {
                ColorConfigOptions a;
                ColorConfigOptions b;
                TLRENDER_ASSERT(a == b);
                a.fileName = "fileName";
                TLRENDER_ASSERT(a != b);
            }
            {
                ColorConfigOptions value;
                value.fileName = "fileName";
                value.input = "input";
                value.display = "display";
                value.view = "view";
                value.look = "look";
                nlohmann::json json;
                to_json(json, value);
                ColorConfigOptions value2;
                from_json(json, value2);
                TLRENDER_ASSERT(value2 == value);
            }
        }
    }
}
