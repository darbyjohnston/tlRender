// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class UtilTest : public tests::ITest
        {
        protected:
            UtilTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<UtilTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _enums();
            void _extensions();
            void _ranges();
            void _loop();
            void _util();
            void _audio();
            void _otioz();
        };
    }
}
