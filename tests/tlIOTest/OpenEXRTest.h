// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class OpenEXRTest : public tests::ITest
        {
        protected:
            OpenEXRTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<OpenEXRTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _enums();
            void _util();
            void _io();
        };
    }
}
