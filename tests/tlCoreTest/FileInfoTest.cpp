// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileInfoTest.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>

#include <dtk/core/FileIO.h>

#include <cstdio>
#include <sstream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        FileInfoTest::FileInfoTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "core_tests::FileInfoTest")
        {}

        std::shared_ptr<FileInfoTest> FileInfoTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<FileInfoTest>(new FileInfoTest(context));
        }

        void FileInfoTest::run()
        {
            _enums();
            _ctors();
            _sequence();
            _list();
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
                DTK_ASSERT(f.getPath().isEmpty());
            }
            {
                const Path path("tmp");
                {
                    dtk::FileIO::create(path.get(), dtk::FileMode::Write);
                }
                const FileInfo f(path);
                DTK_ASSERT(path == f.getPath());
                DTK_ASSERT(Type::File == f.getType());
                DTK_ASSERT(0 == f.getSize());
                DTK_ASSERT(f.getPermissions() != 0);
                DTK_ASSERT(f.getTime() != 0);
                rm(path.get());
            }
        }

        void FileInfoTest::_sequence()
        {
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.1.exr")));
                f.sequence(FileInfo(Path("test.2.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(0, 2));
            }
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.0001.exr")));
                f.sequence(FileInfo(Path("test.0002.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(0, 0));
            }
            {
                FileInfo f(Path("test.0000.exr"));
                f.sequence(FileInfo(Path("test.1.exr")));
                f.sequence(FileInfo(Path("test.2.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(0, 0));
            }
            {
                FileInfo f(Path("test.0.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(0, 0));
            }
            {
                FileInfo f(Path("test.1.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(1, 1));
            }
            {
                FileInfo f(Path("test.exr"));
                f.sequence(FileInfo(Path("test3.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(0, 0));
            }
            {
                FileInfo f(Path("test3.exr"));
                f.sequence(FileInfo(Path("test.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(3, 3));
            }
            {
                FileInfo f(Path("test0999.exr"));
                f.sequence(FileInfo(Path("test1000.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(999, 1000));
            }
            {
                FileInfo f(Path("0001.exr"));
                f.sequence(FileInfo(Path("7800.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(1, 7800));
            }
            {
                FileInfo f(Path("1000.exr"));
                f.sequence(FileInfo(Path("0999.exr")));
                DTK_ASSERT(f.getPath().getSequence() == dtk::RangeI(999, 1000));
                DTK_ASSERT(f.getPath().getPadding() == 4);
                std::string s = f.getPath().get(999);
                DTK_ASSERT("0999.exr" == s);
                s = f.getPath().get(1000);
                DTK_ASSERT("1000.exr" == s);
            }
        }

        void FileInfoTest::_list()
        {
            {
                ListOptions options;
                options.sort = ListSort::Time;
                DTK_ASSERT(options == options);
                DTK_ASSERT(options != ListOptions());
            }
            
            std::string tmp = createTempDir();
            mkdir(file::Path(tmp, "dir").get());
            dtk::FileIO::create(file::Path(tmp, "file.txt").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.1.exr").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.2.exr").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.3.exr").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.1.tif").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.2.tif").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.3.tif").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.0001.tif").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.0002.tif").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "render.0003.tif").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "movie.1.mov").get(), dtk::FileMode::Write);
            dtk::FileIO::create(file::Path(tmp, "movie.2.mov").get(), dtk::FileMode::Write);
            
            {
                std::vector<FileInfo> list;
                ListOptions options;
                options.sequence = true;
                options.sequenceExtensions = { ".exr", ".tif" };
                file::list(tmp, list, options);
                DTK_ASSERT(7 == list.size());
                for (size_t i = 0; i < list.size(); ++i)
                {
                    const auto& path = list[i].getPath();
                    if ("render." == path.getBaseName())
                    {
                        DTK_ASSERT(path.isSequence());
                        DTK_ASSERT(path.getSequence() == dtk::RangeI(1, 3));
                    }
                }
                for (const auto i : { "movie.1.mov", "movie.2.mov" })
                {
                    const auto j = std::find_if(
                        list.begin(),
                        list.end(),
                        [i](const FileInfo& value)
                        {
                            return i == value.getPath().get(-1, PathType::FileName);
                        });
                    DTK_ASSERT(j != list.end());
                }
                
                options.sequence = false;
                file::list(tmp, list, options);
                DTK_ASSERT(13 == list.size());
                for (size_t i = 0; i < list.size(); ++i)
                {
                    const auto& path = list[i].getPath();
                    if ("render." == path.getBaseName())
                    {
                        DTK_ASSERT(!path.isSequence());
                    }
                }
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
                file::list(tmp, list, options);
            }
        }
    }
}
