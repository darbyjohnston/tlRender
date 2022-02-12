// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/PathTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Path.h>

using namespace tl::core;
using namespace tl::file;

namespace tl
{
    namespace CoreTest
    {
        PathTest::PathTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::PathTest", context)
        {}

        std::shared_ptr<PathTest> PathTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<PathTest>(new PathTest(context));
        }

        void PathTest::run()
        {
            {
                Path path;
                TLRENDER_ASSERT(path.isEmpty());
                TLRENDER_ASSERT(path.getDirectory().empty());
                TLRENDER_ASSERT(path.getBaseName().empty());
                TLRENDER_ASSERT(path.getNumber().empty());
                TLRENDER_ASSERT(path.getExtension().empty());
            }
            {
                TLRENDER_ASSERT(Path("/tmp/file.txt").get() == "/tmp/file.txt");
                TLRENDER_ASSERT(Path("/tmp", "file.txt").get() == "/tmp/file.txt");
                TLRENDER_ASSERT(Path("/tmp/", "file.txt").get() == "/tmp/file.txt");
                TLRENDER_ASSERT(Path("\\tmp\\file.txt").get() == "/tmp/file.txt");
            }
            {
                struct Data
                {
                    std::string fileName;
                    std::string directory;
                    std::string baseName;
                    std::string number;
                    int padding = 0;
                    std::string extension;
                };
                const std::vector<Data> data =
                {
                    { "", "", "", "", 0, "" },
                    { "file", "", "file", "", 0, "" },
                    { "file.txt", "", "file", "", 0, ".txt" },
                    { "/tmp/file.txt", "/tmp/", "file", "", 0, ".txt" },
                    { "/tmp/render.1.exr", "/tmp/", "render.", "1", 0, ".exr" },
                    { "/tmp/render.0001.exr", "/tmp/", "render.", "0001", 4, ".exr" },
                    { "/tmp/render0001.exr", "/tmp/", "render", "0001", 4, ".exr" }
                };
                for (const auto& i : data)
                {
                    Path path(i.fileName);
                    TLRENDER_ASSERT(i.fileName == path.get());
                    TLRENDER_ASSERT(i.directory == path.getDirectory());
                    TLRENDER_ASSERT(i.baseName == path.getBaseName());
                    TLRENDER_ASSERT(i.number == path.getNumber());
                    TLRENDER_ASSERT(i.padding == path.getPadding());
                    TLRENDER_ASSERT(i.extension == path.getExtension());
                }
            }
            {
                TLRENDER_ASSERT(Path("/").isAbsolute());
                TLRENDER_ASSERT(Path("/tmp").isAbsolute());
                TLRENDER_ASSERT(Path("\\").isAbsolute());
                TLRENDER_ASSERT(Path("C:").isAbsolute());
                TLRENDER_ASSERT(Path("C:\\tmp").isAbsolute());
                TLRENDER_ASSERT(!Path("").isAbsolute());
                TLRENDER_ASSERT(!Path("../..").isAbsolute());
                TLRENDER_ASSERT(!Path("..\\..").isAbsolute());
            }
        }
    }
}
