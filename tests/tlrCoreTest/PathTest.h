// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class PathTest : public Test::ITest
        {
        protected:
            PathTest();

        public:
            static std::shared_ptr<PathTest> create();

            void run() override;
        };
    }
}
