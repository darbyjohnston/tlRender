// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class AudioTest : public tests::ITest
        {
        protected:
            AudioTest(const std::shared_ptr<dtk::Context>&);

        public:
            static std::shared_ptr<AudioTest> create(const std::shared_ptr<dtk::Context>&);

            void run() override;

        private:
            void _enums();
            void _types();
            void _audio();
            void _audioSystem();
            void _combine();
            void _mix();
            void _reverse();
            void _convert();
            void _move();
            void _resample();
        };
    }
}
