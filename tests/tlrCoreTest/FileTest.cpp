// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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
        FileTest::FileTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::FileTest", context)
        {}

        std::shared_ptr<FileTest> FileTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<FileTest>(new FileTest(context));
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
