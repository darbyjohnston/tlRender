// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class OCIOOptionsTest : public tests::ITest
        {
        protected:
            OCIOOptionsTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<OCIOOptionsTest> create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    }
}