// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class TimelinePlayerTest : public Test::ITest
        {
        protected:
            TimelinePlayerTest();

        public:
            static std::shared_ptr<TimelinePlayerTest> create();

            void run() override;

        private:
            void _enums();
            void _loopTime();
            void _timelinePlayer();
        };
    }
}
