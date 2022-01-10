// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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
            StringTest(const std::shared_ptr<core::Context>&);

        public:
            static std::shared_ptr<StringTest> create(const std::shared_ptr<core::Context>&);

            void run() override;
        
        private:
            void _split();
            void _case();
            void _util();
            void _convert();
            void _escape();
        };
    }
}
