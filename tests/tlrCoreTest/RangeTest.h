// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class RangeTest : public Test::ITest
        {
        protected:
            RangeTest();

        public:
            static std::shared_ptr<RangeTest> create();

            void run() override;
        };
    }
}
