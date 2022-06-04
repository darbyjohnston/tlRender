// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/VectorTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Vector.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        VectorTest::VectorTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::VectorTest", context)
        {}

        std::shared_ptr<VectorTest> VectorTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<VectorTest>(new VectorTest(context));
        }

        void VectorTest::run()
        {
            {
                Vector2i a;
                Vector2i b;
                TLRENDER_ASSERT(a == b);
                a.x = 1;
                TLRENDER_ASSERT(a != b);
            }
            {
                const Vector2i c(1, 2);
                nlohmann::json json;
                to_json(json, c);
                Vector2i c2;
                from_json(json, c2);
                TLRENDER_ASSERT(c == c2);
            }
            {
                const Vector2i c(1, 2);
                std::stringstream ss;
                ss << c;
                Vector2i c2;
                ss >> c2;
                TLRENDER_ASSERT(c == c2);
            }
        }
    }
}
