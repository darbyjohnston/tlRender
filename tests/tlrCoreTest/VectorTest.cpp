// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/VectorTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Vector.h>

#include <sstream>

using namespace tlr::math;

namespace tlr
{
    namespace CoreTest
    {
        VectorTest::VectorTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::VectorTest", context)
        {}

        std::shared_ptr<VectorTest> VectorTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<VectorTest>(new VectorTest(context));
        }

        void VectorTest::run()
        {
            {
                const Vector2i v;
                TLR_ASSERT(0 == v.x);
                TLR_ASSERT(0 == v.y);
            }
            {
                const Vector3i v;
                TLR_ASSERT(0 == v.x);
                TLR_ASSERT(0 == v.y);
                TLR_ASSERT(0 == v.z);
            }
            {
                const Vector4i v;
                TLR_ASSERT(0 == v.x);
                TLR_ASSERT(0 == v.y);
                TLR_ASSERT(0 == v.z);
                TLR_ASSERT(0 == v.w);
            }
            {
                const Vector2i v(1, 2);
                TLR_ASSERT(1 == v.x);
                TLR_ASSERT(2 == v.y);
            }
            {
                const Vector3i v(1, 2, 3);
                TLR_ASSERT(1 == v.x);
                TLR_ASSERT(2 == v.y);
                TLR_ASSERT(3 == v.z);
            }
            {
                const Vector4i v(1, 2, 3, 4);
                TLR_ASSERT(1 == v.x);
                TLR_ASSERT(2 == v.y);
                TLR_ASSERT(3 == v.z);
                TLR_ASSERT(4 == v.w);
            }
            {
                TLR_ASSERT(Vector2i(1, 2) == Vector2i(1, 2));
                TLR_ASSERT(Vector2i(1, 2) != Vector2i(2, 1));
            }
            {
                TLR_ASSERT(Vector3i(1, 2, 3) == Vector3i(1, 2, 3));
                TLR_ASSERT(Vector3i(1, 2, 3) != Vector3i(3, 2, 1));
            }
            {
                TLR_ASSERT(Vector4i(1, 2, 3, 4) == Vector4i(1, 2, 3, 4));
                TLR_ASSERT(Vector4i(1, 2, 3, 4) != Vector4i(4, 3, 2, 1));
            }
            {
                TLR_ASSERT(Vector2i(1, 2) + 1 == Vector2i(2, 3));
            }
            {
                TLR_ASSERT(Vector3i(1, 2, 3) + 1 == Vector3i(2, 3, 4));
            }
            {
                TLR_ASSERT(Vector4i(1, 2, 3, 4) + 1 == Vector4i(2, 3, 4, 5));
            }
        }
    }
}
