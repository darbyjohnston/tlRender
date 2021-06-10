// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class TimeTest : public Test::ITest
        {
        protected:
            TimeTest();

        public:
            static std::shared_ptr<TimeTest> create();

            void run() override;
        };
    }
}
