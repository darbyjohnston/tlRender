// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class LRUCacheTest : public Test::ITest
        {
        protected:
            LRUCacheTest();

        public:
            static std::shared_ptr<LRUCacheTest> create();

            void run() override;
        };
    }
}
