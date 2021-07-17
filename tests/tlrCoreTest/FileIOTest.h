// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class FileIOTest : public Test::ITest
        {
        protected:
            FileIOTest();

        public:
            static std::shared_ptr<FileIOTest> create();

            void run() override;

        private:
            std::string _fileName;
            std::string _text;
            std::string _text2;
        };
    }
}
