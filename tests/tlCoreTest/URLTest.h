// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class URLTest : public tests::ITest
        {
        protected:
            URLTest(const std::shared_ptr<dtk::Context>&);

        public:
            static std::shared_ptr<URLTest> create(const std::shared_ptr<dtk::Context>&);

            void run() override;
            
        private:
            void _util();
            void _encode();
        };
    }
}
