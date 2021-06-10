// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class MatrixTest : public Test::ITest
        {
        protected:
            MatrixTest();

        public:
            static std::shared_ptr<MatrixTest> create();

            void run() override;
        };
    }
}
