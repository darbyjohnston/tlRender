// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class DPXTest : public tests::ITest
        {
        protected:
            DPXTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<DPXTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _enums();
            void _io();
        };
    }
}
