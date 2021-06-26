// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTestLib/ITest.h>

namespace tlr
{
    namespace CoreTest
    {
        class FileTest : public Test::ITest
        {
        protected:
            FileTest();

        public:
            static std::shared_ptr<FileTest> create();

            void run() override;

        private:
            void _func();
            void _io();

            std::string _fileName;
            std::string _text;
            std::string _text2;
        };
    }
}
