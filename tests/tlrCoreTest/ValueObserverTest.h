// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
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
            ValueObserverTest();

        public:
            static std::shared_ptr<ValueObserverTest> create();

            void run() override;
        };
    }
}
