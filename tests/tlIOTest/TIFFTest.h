// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class TIFFTest : public tests::ITest
        {
        protected:
            TIFFTest(const std::shared_ptr<dtk::Context>&);

        public:
            static std::shared_ptr<TIFFTest> create(const std::shared_ptr<dtk::Context>&);

            void run() override;
        };
    }
}
