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
            class TimelinePlayerTest : public Test::ITest
            {
            protected:
                TimelinePlayerTest(const std::shared_ptr<core::system::Context>&);

            public:
                static std::shared_ptr<TimelinePlayerTest> create(const std::shared_ptr<core::system::Context>&);

                void run() override;

            private:
                void _enums();
                void _loop();
                void _timelinePlayer();
            };
        }
    }
}
