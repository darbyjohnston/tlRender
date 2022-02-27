// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace tests
    {
        namespace io_test
        {
            class DPXTest : public Test::ITest
            {
            protected:
                DPXTest(const std::shared_ptr<core::Context>&);

            public:
                static std::shared_ptr<DPXTest> create(const std::shared_ptr<core::Context>&);

                void run() override;

            private:
                void _enums();
                void _io();
            };
        }
    }
}
