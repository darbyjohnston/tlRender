// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileInfoTest.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
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
            _ctors();
            _sequence();
            _list();
            _serialize();
        }

        void FileInfoTest::_enums()
        {
            _enum<Type>("Type", getTypeEnums);
            _enum<ListSort>("ListSort", getListSortEnums);
        }

        void FileInfoTest::_ctors()
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
                rm(path.get());
            }
        }

        void FileInfoTest::_sequence()
        {
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.1.exr")));
                f.sequence(FileInfo(Path("test.2.exr")));
                TLRENDER_ASSERT(f.getPath().getSequence() == math::IntRange(0, 2));
            }
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.0001.exr")));
                f.sequence(FileInfo(Path("test.0002.exr")));
                TLRENDER_ASSERT(f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test.0000.exr"));
                f.sequence(FileInfo(Path("test.1.exr")));
                f.sequence(FileInfo(Path("test.2.exr")));
                TLRENDER_ASSERT(f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                TLRENDER_ASSERT(f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test.1.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                TLRENDER_ASSERT(f.getPath().getSequence() == math::IntRange(1, 1));
            }
            {
                FileInfo f(Path("test.exr"));
                f.sequence(FileInfo(Path("test3.exr")));
                TLRENDER_ASSERT(f.getPath().getSequence() == math::IntRange(0, 0));
            }
            {
                FileInfo f(Path("test3.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                TLRENDER_ASSERT(f.getPath().getSequence() == math::IntRange(3, 3));
            }
        }

        void FileInfoTest::_list()
        {
            {
                ListOptions options;
                options.sort = ListSort::Time;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != ListOptions());
            }
            std::vector<ListOptions> optionsList;
            for (auto sort : getListSortEnums())
            {
                ListOptions options;
                options.sort = sort;
                optionsList.push_back(options);
            }
            {
                ListOptions options;
                options.reverseSort = true;
                optionsList.push_back({ options });
            }
            {
                ListOptions options;
                options.sortDirectoriesFirst = false;
                optionsList.push_back({ options });
            }
            {
                ListOptions options;
                options.sequence = false;
                optionsList.push_back({ options });
            }
            for (const auto& options : optionsList)
            {
                std::vector<FileInfo> list;
                file::list(".", list, options);
            }
        }

        void FileInfoTest::_serialize()
        {
            {
                ListOptions options;
                options.sort = ListSort::Time;
                nlohmann::json json;
                to_json(json, options);
                ListOptions options2;
                from_json(json, options2);
                TLRENDER_ASSERT(options == options2);
            }
        }
    }
}
