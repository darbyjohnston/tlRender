// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class STBTest : public tests::ITest
        {
        protected:
            STBTest(const std::shared_ptr<feather_tk::Context>&);

        public:
            static std::shared_ptr<STBTest> create(const std::shared_ptr<feather_tk::Context>&);

            void run() override;

        private:
            void _io();
        };
    }
}
