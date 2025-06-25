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
        HDRTest::HDRTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "core_tests::HDRTest")
        {}

        std::shared_ptr<HDRTest> HDRTest::create(const std::shared_ptr<feather_tk::Context>& context)
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
                FEATHER_TK_ASSERT(a == b);
                a.eotf = image::HDR_EOTF::ST2084;
                FEATHER_TK_ASSERT(a != b);
            }
        }

        void HDRTest::_serialize()
        {
            {
                HDRData value;
                value.eotf = image::HDR_EOTF::ST2084;
                value.primaries[0].x = .1F;
                value.primaries[0].y = .2F;
                value.primaries[1].x = .3F;
                value.primaries[1].y = .4F;
                value.primaries[2].x = .5F;
                value.primaries[2].y = .6F;
                value.primaries[3].x = .7F;
                value.primaries[3].y = .8F;
                value.displayMasteringLuminance = feather_tk::RangeF(.1F, .2F);
                value.maxCLL = .1F;
                value.maxFALL = .2F;
                nlohmann::json json;
                to_json(json, value);
                HDRData value2;
                from_json(json, value2);
                FEATHER_TK_ASSERT(value == value2);
            }
        }
    }
}
