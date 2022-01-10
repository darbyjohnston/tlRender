// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class ValueObserverTest : public Test::ITest
        {
        protected:
            ValueObserverTest(const std::shared_ptr<core::Context>&);

        public:
            static std::shared_ptr<ValueObserverTest> create(const std::shared_ptr<core::Context>&);

            void run() override;
        };
    }
}
