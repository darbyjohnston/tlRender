// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class PathTest : public tests::ITest
        {
        protected:
            PathTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<PathTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;

        private:
            void _enums();
            void _path();
            void _util();
        };
    }
}
