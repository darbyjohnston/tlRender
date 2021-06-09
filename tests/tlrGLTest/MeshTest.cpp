// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrGLTest/MeshTest.h>

namespace tlr
{
    namespace GLTest
    {
        MeshTest::MeshTest() :
            ITest("GLTest::MeshTest")
        {}

        std::shared_ptr<MeshTest> MeshTest::create()
        {
            return std::shared_ptr<MeshTest>(new MeshTest);
        }

        void MeshTest::run()
        {
        }
    }
}
