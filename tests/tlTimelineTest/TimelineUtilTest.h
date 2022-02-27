// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace tests
    {
        namespace timeline_test
        {
            class TimelineUtilTest : public Test::ITest
            {
            protected:
                TimelineUtilTest(const std::shared_ptr<core::Context>&);

            public:
                static std::shared_ptr<TimelineUtilTest> create(const std::shared_ptr<core::Context>&);

                void run() override;

            private:
                void _enums();
                void _ranges();
                void _util();
            };
        }
    }
}

