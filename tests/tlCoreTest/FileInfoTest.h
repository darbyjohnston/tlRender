// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class FileInfoTest : public tests::ITest
        {
        protected:
            FileInfoTest(const std::shared_ptr<dtk::Context>&);

        public:
            static std::shared_ptr<FileInfoTest> create(const std::shared_ptr<dtk::Context>&);

            void run() override;

        private:
            void _enums();
            void _ctors();
            void _sequence();
            void _list();
        };
    }
}
