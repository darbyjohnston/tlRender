// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class FFmpegTest : public Test::ITest
        {
        protected:
            FFmpegTest();

        public:
            static std::shared_ptr<FFmpegTest> create();

            void run() override;

        private:
            void _enums();
            void _io();
        };
    }
}
