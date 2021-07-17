// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace GLTest
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
