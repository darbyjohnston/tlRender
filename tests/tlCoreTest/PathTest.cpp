// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCoreTest/PathTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Path.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <iostream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        PathTest::PathTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::PathTest", context)
        {}

        std::shared_ptr<PathTest> PathTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PathTest>(new PathTest(context));
        }

        void PathTest::run()
        {
            {
                PathOptions a;
                PathOptions b;
                TLRENDER_ASSERT(a == b);
                a.maxNumberDigits = 0;
                TLRENDER_ASSERT(a != b);
            }
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
                TLRENDER_ASSERT(Path("\\tmp\\file.txt").get() == "\\tmp\\file.txt");
            }
            {
                TLRENDER_ASSERT(Path("/tmp/", "render.", "0001", 4, ".exr").get() ==
                    "/tmp/render.0001.exr");
                TLRENDER_ASSERT(Path("/tmp/", "render.", "0001", 4, ".exr").get(2) ==
                    "/tmp/render.0002.exr");
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
            {
                Path a("/");
                Path b("/");
                TLRENDER_ASSERT(a == b);
                b = Path("/tmp");
                TLRENDER_ASSERT(a != b);
            }
            {
                const auto drives = getDrives();
                _print(string::Format("Drives: {0}").arg(string::join(drives, ", ")));
            }
        }
    }
}
