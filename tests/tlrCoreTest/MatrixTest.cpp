// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/MatrixTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Matrix.h>

using namespace tlr::math;

namespace tlr
{
    namespace CoreTest
    {
        MatrixTest::MatrixTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::MatrixTest", context)
        {}

        std::shared_ptr<MatrixTest> MatrixTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<MatrixTest>(new MatrixTest(context));
        }

        void MatrixTest::run()
        {
            {
                Matrix3x3f m;

                TLR_ASSERT(1.F == m.v[0]);
                TLR_ASSERT(0.F == m.v[1]);
                TLR_ASSERT(0.F == m.v[2]);

                TLR_ASSERT(0.F == m.v[3]);
                TLR_ASSERT(1.F == m.v[4]);
                TLR_ASSERT(0.F == m.v[5]);

                TLR_ASSERT(0.F == m.v[6]);
                TLR_ASSERT(0.F == m.v[7]);
                TLR_ASSERT(1.F == m.v[8]);
            }
            {
                Matrix3x3f m(
                    0.F, 1.F, 2.F,
                    3.F, 4.F, 5.F,
                    6.F, 7.F, 8.F);

                TLR_ASSERT(0.F == m.v[0]);
                TLR_ASSERT(1.F == m.v[1]);
                TLR_ASSERT(2.F == m.v[2]);

                TLR_ASSERT(3.F == m.v[3]);
                TLR_ASSERT(4.F == m.v[4]);
                TLR_ASSERT(5.F == m.v[5]);

                TLR_ASSERT(6.F == m.v[6]);
                TLR_ASSERT(7.F == m.v[7]);
                TLR_ASSERT(8.F == m.v[8]);
            }
            {
                Matrix4x4f m;

                TLR_ASSERT(1.F == m.v[0]);
                TLR_ASSERT(0.F == m.v[1]);
                TLR_ASSERT(0.F == m.v[2]);
                TLR_ASSERT(0.F == m.v[3]);

                TLR_ASSERT(0.F == m.v[4]);
                TLR_ASSERT(1.F == m.v[5]);
                TLR_ASSERT(0.F == m.v[6]);
                TLR_ASSERT(0.F == m.v[7]);

                TLR_ASSERT(0.F == m.v[8]);
                TLR_ASSERT(0.F == m.v[9]);
                TLR_ASSERT(1.F == m.v[10]);
                TLR_ASSERT(0.F == m.v[11]);

                TLR_ASSERT(0.F == m.v[12]);
                TLR_ASSERT(0.F == m.v[13]);
                TLR_ASSERT(0.F == m.v[14]);
                TLR_ASSERT(1.F == m.v[15]);
            }
            {
                Matrix4x4f m(
                    0.F, 1.F, 2.F, 3.F,
                    4.F, 5.F, 6.F, 7.F,
                    8.F, 9.F, 10.F, 11.F,
                    12.F, 13.F, 14.F, 15.F);

                TLR_ASSERT(0.F == m.v[0]);
                TLR_ASSERT(1.F == m.v[1]);
                TLR_ASSERT(2.F == m.v[2]);
                TLR_ASSERT(3.F == m.v[3]);

                TLR_ASSERT(4.F == m.v[4]);
                TLR_ASSERT(5.F == m.v[5]);
                TLR_ASSERT(6.F == m.v[6]);
                TLR_ASSERT(7.F == m.v[7]);

                TLR_ASSERT(8.F == m.v[8]);
                TLR_ASSERT(9.F == m.v[9]);
                TLR_ASSERT(10.F == m.v[10]);
                TLR_ASSERT(11.F == m.v[11]);

                TLR_ASSERT(12.F == m.v[12]);
                TLR_ASSERT(13.F == m.v[13]);
                TLR_ASSERT(14.F == m.v[14]);
                TLR_ASSERT(15.F == m.v[15]);
            }
            {
                const auto m = ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

                TLR_ASSERT(1.F == m.v[0]);
                TLR_ASSERT(0.F == m.v[1]);
                TLR_ASSERT(0.F == m.v[2]);
                TLR_ASSERT(0.F == m.v[3]);

                TLR_ASSERT(0.F == m.v[4]);
                TLR_ASSERT(1.F == m.v[5]);
                TLR_ASSERT(0.F == m.v[6]);
                TLR_ASSERT(0.F == m.v[7]);

                TLR_ASSERT(0.F == m.v[8]);
                TLR_ASSERT(0.F == m.v[9]);
                TLR_ASSERT(-1.F == m.v[10]);
                TLR_ASSERT(0.F == m.v[11]);

                TLR_ASSERT(0.F == m.v[12]);
                TLR_ASSERT(0.F == m.v[13]);
                TLR_ASSERT(0.F == m.v[14]);
                TLR_ASSERT(1.F == m.v[15]);
            }
            {
                Matrix3x3f a(
                    0.F, 1.F, 2.F,
                    3.F, 4.F, 5.F,
                    6.F, 7.F, 8.F);
                Matrix3x3f b(
                    8.F, 7.F, 6.F,
                    5.F, 4.F, 3.F,
                    2.F, 1.F, 0.F);

                const auto c = a * b;

                TLR_ASSERT(57.F == c.v[0]);
                TLR_ASSERT(78.F == c.v[1]);
                TLR_ASSERT(99.F == c.v[2]);

                TLR_ASSERT(30.F == c.v[3]);
                TLR_ASSERT(42.F == c.v[4]);
                TLR_ASSERT(54.F == c.v[5]);

                TLR_ASSERT(3.F == c.v[6]);
                TLR_ASSERT(6.F == c.v[7]);
                TLR_ASSERT(9.F == c.v[8]);
            }
            {
                Matrix4x4f a(
                    0.F, 1.F, 2.F, 3.F,
                    4.F, 5.F, 6.F, 7.F,
                    8.F, 9.F, 10.F, 11.F,
                    12.F, 13.F, 14.F, 15.F);
                Matrix4x4f b(
                    15.F, 14.F, 13.F, 12.F,
                    11.F, 10.F, 9.F, 8.F,
                    7.F, 6.F, 5.F, 4.F,
                    3.F, 2.F, 1.F, 0.F);

                const auto c = a * b;

                TLR_ASSERT(304.F == c.v[0]);
                TLR_ASSERT(358.F == c.v[1]);
                TLR_ASSERT(412.F == c.v[2]);
                TLR_ASSERT(466.F == c.v[3]);

                TLR_ASSERT(208.F == c.v[4]);
                TLR_ASSERT(246.F == c.v[5]);
                TLR_ASSERT(284.F == c.v[6]);
                TLR_ASSERT(322.F == c.v[7]);

                TLR_ASSERT(112.F == c.v[8]);
                TLR_ASSERT(134.F == c.v[9]);
                TLR_ASSERT(156.F == c.v[10]);
                TLR_ASSERT(178.F == c.v[11]);

                TLR_ASSERT(16.F == c.v[12]);
                TLR_ASSERT(22.F == c.v[13]);
                TLR_ASSERT(28.F == c.v[14]);
                TLR_ASSERT(34.F == c.v[15]);
            }
        }
    }
}
