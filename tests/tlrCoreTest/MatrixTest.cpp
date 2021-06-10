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
        MatrixTest::MatrixTest() :
            ITest("CoreTest::MatrixTest")
        {}

        std::shared_ptr<MatrixTest> MatrixTest::create()
        {
            return std::shared_ptr<MatrixTest>(new MatrixTest);
        }

        void MatrixTest::run()
        {
            {
                const Matrix3x3f m;

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
                const Matrix4x4f m;

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
        }
    }
}
