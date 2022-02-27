// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace tests
    {
        namespace core_test
        {
            class AudioTest : public Test::ITest
            {
            protected:
                AudioTest(const std::shared_ptr<core::system::Context>&);

            public:
                static std::shared_ptr<AudioTest> create(const std::shared_ptr<core::system::Context>&);

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
}
