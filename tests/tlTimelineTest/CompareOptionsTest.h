// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class CompareOptionsTest : public tests::ITest
        {
        protected:
            CompareOptionsTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<CompareOptionsTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
