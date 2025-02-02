// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class SizeTest : public tests::ITest
        {
        protected:
            SizeTest(const std::shared_ptr<dtk::Context>&);

        public:
            static std::shared_ptr<SizeTest> create(const std::shared_ptr<dtk::Context>&);

            void run() override;

        private:
            void _ctors();
            void _components();
            void _dimensions();
            void _operators();
            void _serialize();
        };
    }
}
