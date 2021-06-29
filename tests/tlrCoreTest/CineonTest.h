// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class CineonTest : public Test::ITest
        {
        protected:
            CineonTest();

        public:
            static std::shared_ptr<CineonTest> create();

            void run() override;
            
        private:
            void _enums();
            void _io();
        };
    }
}
