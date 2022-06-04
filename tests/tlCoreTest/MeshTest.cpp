// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/MeshTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Mesh.h>

#include <sstream>

using namespace tl::geom;

namespace tl
{
    namespace core_tests
    {
        MeshTest::MeshTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::MeshTest", context)
        {}

        std::shared_ptr<MeshTest> MeshTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MeshTest>(new MeshTest(context));
        }

        void MeshTest::run()
        {
            {
                const math::Vector2f p(0.F, 0.F);
                const math::Vector2f v0(1.F, 0.F);
                const math::Vector2f v1(0.F, 1.F);
                TLRENDER_ASSERT(edge(p, v0, v1) == -1.F);
                TLRENDER_ASSERT(edge(p, v1, v0) == 1.F);
            }
            {
                const auto m = createSphere(1.F, 10, 10);
                TLRENDER_ASSERT(!m.triangles.empty());
            }
        }
    }
}
