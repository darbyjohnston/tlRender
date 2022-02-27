// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace tests
    {
        namespace render_gl_test
        {
            class MeshTest : public Test::ITest
            {
            protected:
                MeshTest(const std::shared_ptr<core::Context>&);

            public:
                static std::shared_ptr<MeshTest> create(const std::shared_ptr<core::Context>&);

                void run() override;
            };
        }
    }
}
