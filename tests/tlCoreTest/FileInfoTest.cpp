// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileInfoTest.h>

#include <tlCore/Assert.h>
#include <tlCore/FileIO.h>
#include <tlCore/FileInfo.h>

#include <cstdio>
#include <sstream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        FileInfoTest::FileInfoTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::FileInfoTest", context)
        {}

        std::shared_ptr<FileInfoTest> FileInfoTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FileInfoTest>(new FileInfoTest(context));
        }

        void FileInfoTest::run()
        {
            _enums();
            _tests();
        }

        void FileInfoTest::_enums()
        {
            _enum<Type>("Type", getTypeEnums);
        }

        void FileInfoTest::_tests()
        {
            {
                const FileInfo f;
                TLRENDER_ASSERT(f.getPath().isEmpty());
            }
            {
                const Path path("tmp");
                {
                    FileIO::create(path.get(), Mode::Write);
                }
                const FileInfo f(path);
                TLRENDER_ASSERT(path == f.getPath());
                TLRENDER_ASSERT(Type::File == f.getType());
                TLRENDER_ASSERT(0 == f.getSize());
                TLRENDER_ASSERT(f.getPermissions() != 0);
                TLRENDER_ASSERT(f.getTime() != 0);
                TLRENDER_ASSERT(0 == remove(path.get().c_str()));
            }
            {
                const Path path("tmp");
                const FileInfo f(path);
            }
            {
                for (const auto& i : dirList("."))
                {
                    _print(i.getPath().get());
                }
            }
        }
    }
}
