// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class PNGTest : public Test::ITest
        {
        protected:
            PNGTest();

        public:
            static std::shared_ptr<PNGTest> create();

            void run() override;
        };
    }
}
