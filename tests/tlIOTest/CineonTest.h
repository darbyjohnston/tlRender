// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class CineonTest : public tests::ITest
        {
        protected:
            CineonTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<CineonTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _enums();
            void _io();
        };
    }
}
