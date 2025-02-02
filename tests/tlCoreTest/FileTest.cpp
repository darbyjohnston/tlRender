// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileTest.h>

#include <tlCore/File.h>
#include <tlCore/FileIO.h>

#include <dtk/core/Format.h>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        FileTest::FileTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "core_tests::FileTest")
        {}

        std::shared_ptr<FileTest> FileTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<FileTest>(new FileTest(context));
        }

        void FileTest::run()
        {
            _file();
            _dir();
            _temp();
        }

        void FileTest::_file()
        {
            const std::string fileName = "File Test";
            {
                FileIO::create(fileName, Mode::Write);
            }
            DTK_ASSERT(exists(fileName));
            DTK_ASSERT(rm(fileName));
        }

        void FileTest::_dir()
        {
            {
                bool r = mkdir("File Test");
                DTK_ASSERT(r);
                r = mkdir("File Test");
                DTK_ASSERT(!r);
                r = rmdir("File Test");
                DTK_ASSERT(r);
                r = rmdir("File Test");
                DTK_ASSERT(!r);
            }
            {
                _print(dtk::Format("CWD: {0}").arg(getCWD()));
            }
        }

        void FileTest::_temp()
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
