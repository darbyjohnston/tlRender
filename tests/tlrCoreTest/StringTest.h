// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class StringTest : public Test::ITest
        {
        protected:
            StringTest();

        public:
            static std::shared_ptr<StringTest> create();

            void run() override;
        };
    }
}
