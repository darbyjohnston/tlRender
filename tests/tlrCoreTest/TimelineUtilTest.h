// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
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
