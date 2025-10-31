// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class SGITest : public tests::ITest
        {
        protected:
            SGITest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<SGITest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _io();
        };
    }
}
