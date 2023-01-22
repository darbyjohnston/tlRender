// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
            AudioTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<AudioTest> create(const std::shared_ptr<system::Context>&);

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
