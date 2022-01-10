// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class AudioTest : public Test::ITest
        {
        protected:
            AudioTest(const std::shared_ptr<core::Context>&);

        public:
            static std::shared_ptr<AudioTest> create(const std::shared_ptr<core::Context>&);

            void run() override;

        private:
            void _enums();
            void _types();
            void _audio();
            void _audioSystem();
            void _mix();
            void _convert();
            void _interleave();
            void _copy();
        };
    }
}
