// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class TimelineTest : public Test::ITest
        {
        protected:
            TimelineTest(const std::shared_ptr<core::Context>&);

        public:
            static std::shared_ptr<TimelineTest> create(const std::shared_ptr<core::Context>&);

            void run() override;

        private:
            void _enums();
            void _ranges();
            void _transitions();
            void _frames();
            void _timeline();
        };
    }
}
