// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLTest/MeshTest.h>

namespace tl
{
    namespace gl_tests
    {
        MeshTest::MeshTest(const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::MeshTest", context)
        {}

        std::shared_ptr<MeshTest> MeshTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MeshTest>(new MeshTest(context));
        }

        void MeshTest::run()
        {
        }
    }
}
