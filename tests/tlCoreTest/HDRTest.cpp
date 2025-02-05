// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/HDRTest.h>

#include <tlCore/HDR.h>

using namespace tl::image;

namespace tl
{
    namespace core_tests
    {
        HDRTest::HDRTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "core_tests::HDRTest")
        {}

        std::shared_ptr<HDRTest> HDRTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<HDRTest>(new HDRTest(context));
        }

        void HDRTest::run()
        {
            _enums();
            _operators();
            _serialize();
        }

        void HDRTest::_enums()
        {
            _enum<HDR_EOTF>("HDR_EOTF", getHDR_EOTFEnums);
            _enum<HDRPrimaries>("HDRPrimaries", getHDRPrimariesEnums);
        }

        void HDRTest::_operators()
        {
            {
                HDRData a;
                HDRData b;
                DTK_ASSERT(a == b);
                a.eotf = image::HDR_EOTF::ST2084;
                DTK_ASSERT(a != b);
            }
        }

        void HDRTest::_serialize()
        {
            {
                HDRData value;
                value.eotf = image::HDR_EOTF::ST2084;
                value.primaries[0].x = .1F;
                value.primaries[0].y = 1.F;
                value.primaries[1].x = .2F;
                value.primaries[1].y = .9F;
                value.primaries[2].x = .3F;
                value.primaries[2].y = .8F;
                value.primaries[3].x = .4F;
                value.primaries[3].y = .7F;
                value.displayMasteringLuminance = dtk::RangeF(.5F, .6F);
                value.maxCLL = 0.1F;
                value.maxFALL = 0.2F;
                nlohmann::json json;
                to_json(json, value);
                HDRData value2;
                from_json(json, value2);
                DTK_ASSERT(value == value2);
            }
        }
    }
}
