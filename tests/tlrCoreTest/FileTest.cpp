// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/FileTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>

#include <sstream>

using namespace tlr::file;

namespace tlr
{
    namespace CoreTest
    {
        FileTest::FileTest() :
            ITest("CoreTest::FileTest")
        {}

        std::shared_ptr<FileTest> FileTest::create()
        {
            return std::shared_ptr<FileTest>(new FileTest);
        }

        void FileTest::run()
        {
            {
                std::stringstream ss;
                ss << "Temp dir:" << createTempDir();
                _print(ss.str());
            }
        }
    }
}
