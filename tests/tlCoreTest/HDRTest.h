// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class HDRTest : public tests::ITest
        {
        protected:
            HDRTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<HDRTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _enums();
            void _operators();
            void _serialize();
        };
    }
}
