// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class STBTest : public tests::ITest
        {
        protected:
            STBTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<STBTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _io();
        };
    }
}
