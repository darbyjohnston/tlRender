// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class TimeTest : public tests::ITest
        {
        protected:
            TimeTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<TimeTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
            
        private:
            void _otime();
            void _util();
            void _keycode();
            void _timecode();
            void _serialize();
        };
    }
}
