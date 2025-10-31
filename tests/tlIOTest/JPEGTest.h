// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class JPEGTest : public tests::ITest
        {
        protected:
            JPEGTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<JPEGTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
