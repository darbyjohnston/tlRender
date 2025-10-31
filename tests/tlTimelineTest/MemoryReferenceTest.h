// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class MemoryReferenceTest : public tests::ITest
        {
        protected:
            MemoryReferenceTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<MemoryReferenceTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
