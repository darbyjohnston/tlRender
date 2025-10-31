// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class URLTest : public tests::ITest
        {
        protected:
            URLTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<URLTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
            
        private:
            void _util();
            void _encode();
        };
    }
}
