// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class CompareOptionsTest : public tests::ITest
        {
        protected:
            CompareOptionsTest(const std::shared_ptr<dtk::Context>&);

        public:
            static std::shared_ptr<CompareOptionsTest> create(const std::shared_ptr<dtk::Context>&);

            void run() override;
        };
    }
}
