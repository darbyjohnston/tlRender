// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlRenderGLTest/MeshTest.h>

namespace tl
{
    namespace tests
    {
        namespace render_gl_test
        {
            MeshTest::MeshTest(const std::shared_ptr<core::Context>& context) :
                ITest("gl_test::MeshTest", context)
            {}

            std::shared_ptr<MeshTest> MeshTest::create(const std::shared_ptr<core::Context>& context)
            {
                return std::shared_ptr<MeshTest>(new MeshTest(context));
            }

            void MeshTest::run()
            {
            }
        }
    }
}
