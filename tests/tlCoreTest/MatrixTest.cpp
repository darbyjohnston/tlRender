// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/MatrixTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Matrix.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        MatrixTest::MatrixTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::MatrixTest", context)
        {}

        std::shared_ptr<MatrixTest> MatrixTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MatrixTest>(new MatrixTest(context));
        }

        void MatrixTest::run()
        {
            {
                Matrix3x3f a;
                Matrix3x3f b;
                TLRENDER_ASSERT(a == b);
                a.e[1] = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                Matrix4x4f a;
                Matrix4x4f b;
                TLRENDER_ASSERT(a == b);
                a.e[1] = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                Matrix3x3f a;
                Matrix3x3f b;
                TLRENDER_ASSERT(a * b == Matrix3x3f());
            }
            {
                Matrix4x4f a;
                Matrix4x4f b;
                TLRENDER_ASSERT(a * b == Matrix4x4f());
            }
            {
                const Matrix3x3f m;
                nlohmann::json json;
                to_json(json, m);
                Matrix3x3f m2;
                from_json(json, m2);
                TLRENDER_ASSERT(m == m2);
            }
            {
                const Matrix4x4f m;
                nlohmann::json json;
                to_json(json, m);
                Matrix4x4f m2;
                from_json(json, m2);
                TLRENDER_ASSERT(m == m2);
            }
            {
                const Matrix3x3f m;
                std::stringstream ss;
                ss << m;
                Matrix3x3f m2(
                    0.F, 0.F, 0.F,
                    0.F, 0.F, 0.F,
                    0.F, 0.F, 0.F);
                ss >> m2;
                TLRENDER_ASSERT(m == m2);
            }
            {
                const Matrix4x4f m;
                std::stringstream ss;
                ss << m;
                Matrix4x4f m2(
                    0.F, 0.F, 0.F, 0.F,
                    0.F, 0.F, 0.F, 0.F,
                    0.F, 0.F, 0.F, 0.F,
                    0.F, 0.F, 0.F, 0.F);
                ss >> m2;
                TLRENDER_ASSERT(m == m2);
            }
            try
            {
                Matrix3x3f m;
                std::stringstream ss("...");
                ss >> m;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                Matrix4x4f m;
                std::stringstream ss("...");
                ss >> m;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
    }
}
