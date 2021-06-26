// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class MemoryTest : public Test::ITest
        {
        protected:
            MemoryTest();

        public:
            static std::shared_ptr<MemoryTest> create();

            void run() override;
        
        private:
            void _enums();
            void _endian();
        };
    }
}
