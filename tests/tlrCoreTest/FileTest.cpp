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
                TLR_ASSERT(isAbsolute("/"));
                TLR_ASSERT(isAbsolute("/tmp"));
                TLR_ASSERT(isAbsolute("C:"));
                TLR_ASSERT(isAbsolute("C:\\tmp"));
                TLR_ASSERT(!isAbsolute(""));
                TLR_ASSERT(!isAbsolute("../.."));
                TLR_ASSERT(!isAbsolute("..\\.."));
            }
            {
                TLR_ASSERT(normalize("/tmp/file.txt") == "/tmp/file.txt");
                TLR_ASSERT(normalize("\\tmp\\file.txt") == "/tmp/file.txt");
            }
            {
                std::string path;
                std::string baseName;
                std::string number;
                std::string extension;
                split("", &path, &baseName, &number, &extension);
                TLR_ASSERT(path.empty());
                TLR_ASSERT(baseName.empty());
                TLR_ASSERT(number.empty());
                TLR_ASSERT(extension.empty());
            }
            {
                std::string path;
                std::string baseName;
                std::string number;
                std::string extension;
                split("/tmp/file.txt", &path, &baseName, &number, &extension);
                TLR_ASSERT("/tmp/" == path);
                TLR_ASSERT("file" == baseName);
                TLR_ASSERT(number.empty());
                TLR_ASSERT(".txt" == extension);
            }
            {
                std::string path;
                std::string baseName;
                std::string number;
                std::string extension;
                split("/tmp/render.0001.exr", &path, &baseName, &number, &extension);
                TLR_ASSERT("/tmp/" == path);
                TLR_ASSERT("render." == baseName);
                TLR_ASSERT("0001" == number);
                TLR_ASSERT(".exr" == extension);
            }
            {
                std::stringstream ss;
                ss << "Temp dir:" << createTempDir();
                _print(ss.str());
            }
        }
    }
}
