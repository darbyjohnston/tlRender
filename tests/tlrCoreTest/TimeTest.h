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
            TimeTest(const std::shared_ptr<core::Context>&);

        public:
            static std::shared_ptr<TimeTest> create(const std::shared_ptr<core::Context>&);

            void run() override;
        };
    }
}
