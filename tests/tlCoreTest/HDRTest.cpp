// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/HDRTest.h>

#include <tlCore/Assert.h>
#include <tlCore/HDR.h>

using namespace tl::imaging;

namespace tl
{
    namespace core_tests
    {
        HDRTest::HDRTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::HDRTest", context)
        {}

        std::shared_ptr<HDRTest> HDRTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<HDRTest>(new HDRTest(context));
        }

        void HDRTest::run()
        {
            {
                imaging::HDR value;
                value.eotf = 1;
                value.redPrimaries.first = .1F;
                value.redPrimaries.second = 1.F;
                value.greenPrimaries.first = .2F;
                value.greenPrimaries.second = .9F;
                value.bluePrimaries.first = .3F;
                value.bluePrimaries.second = .8F;
                value.whitePrimaries.first = .4F;
                value.whitePrimaries.second = .7F;
                value.displayMasteringLuminance = math::FloatRange(.5F, .6F);
                value.maxCLL = 0.1F;
                value.maxFALL = 0.2F;
                nlohmann::json json;
                imaging::to_json(json, value);
                imaging::HDR value2;
                imaging::from_json(json, value2);
                TLRENDER_ASSERT(value2 == value);
            }
        }
    }
}
