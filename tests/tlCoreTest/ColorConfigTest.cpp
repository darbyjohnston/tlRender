// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ColorConfigTest.h>

#include <tlCore/Assert.h>
#include <tlCore/ColorConfig.h>

using namespace tl::imaging;

namespace tl
{
    namespace core_tests
    {
        ColorConfigTest::ColorConfigTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ColorConfigTest", context)
        {}

        std::shared_ptr<ColorConfigTest> ColorConfigTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ColorConfigTest>(new ColorConfigTest(context));
        }

        void ColorConfigTest::run()
        {
            {
                imaging::ColorConfig value;
                value.fileName = "fileName";
                value.input = "input";
                value.display = "display";
                value.view = "view";
                value.look = "look";
                nlohmann::json json;
                imaging::to_json(json, value);
                imaging::ColorConfig value2;
                imaging::from_json(json, value2);
                TLRENDER_ASSERT(value2 == value);
            }
        }
    }
}
