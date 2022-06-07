// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileTest.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/OS.h>

#include <sstream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        FileTest::FileTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::FileTest", context)
        {}

        std::shared_ptr<FileTest> FileTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FileTest>(new FileTest(context));
        }

        void FileTest::run()
        {
            {
                std::stringstream ss;
                ss << "Temp dir: " << getTemp();
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "Temp dir: " << createTempDir();
                _print(ss.str());
            }
        }
    }
}
