// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrGLTest/MeshTest.h>

namespace tlr
{
    namespace GLTest
    {
        MeshTest::MeshTest(const std::shared_ptr<core::Context>& context) :
            ITest("GLTest::MeshTest", context)
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
