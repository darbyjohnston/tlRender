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
                Vector2f a;
                Vector2f b;
                TLRENDER_ASSERT(a == b);
                a.x = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                Vector3f a;
                Vector3f b;
                TLRENDER_ASSERT(a == b);
                a.x = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                Vector4f a;
                Vector4f b;
                TLRENDER_ASSERT(a == b);
                a.x = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                const Vector2i v(1, 2);
                nlohmann::json json;
                to_json(json, v);
                Vector2i v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector2f v(1.F, 2.F);
                nlohmann::json json;
                to_json(json, v);
                Vector2f v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector3f v(1.F, 2.F, 3.F);
                nlohmann::json json;
                to_json(json, v);
                Vector3f v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector4f v(1.F, 2.F, 3.F, 4.F);
                nlohmann::json json;
                to_json(json, v);
                Vector4f v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector2i v(1, 2);
                std::stringstream ss;
                ss << v;
                Vector2i v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector2f v(1.F, 2.F);
                std::stringstream ss;
                ss << v;
                Vector2f v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector3f v(1.F, 2.F, 3.F);
                std::stringstream ss;
                ss << v;
                Vector3f v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector4f v(1.F, 2.F, 3.F, 4.F);
                std::stringstream ss;
                ss << v;
                Vector4f v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            try
            {
                Vector2i v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                Vector2f v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                Vector3f v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                Vector4f v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
    }
}
