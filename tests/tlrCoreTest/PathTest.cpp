// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/PathTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Path.h>

using namespace tlr::core;
using namespace tlr::file;

namespace tlr
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
                TLR_ASSERT(path.isEmpty());
                TLR_ASSERT(path.getDirectory().empty());
                TLR_ASSERT(path.getBaseName().empty());
                TLR_ASSERT(path.getNumber().empty());
                TLR_ASSERT(path.getExtension().empty());
            }
            {
                TLR_ASSERT(Path("/tmp/file.txt").get() == "/tmp/file.txt");
                TLR_ASSERT(Path("\\tmp\\file.txt").get() == "/tmp/file.txt");
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
                    TLR_ASSERT(i.fileName == path.get());
                    TLR_ASSERT(i.directory == path.getDirectory());
                    TLR_ASSERT(i.baseName == path.getBaseName());
                    TLR_ASSERT(i.number == path.getNumber());
                    TLR_ASSERT(i.padding == path.getPadding());
                    TLR_ASSERT(i.extension == path.getExtension());
                }
            }
            {
                TLR_ASSERT(Path("/").isAbsolute());
                TLR_ASSERT(Path("/tmp").isAbsolute());
                TLR_ASSERT(Path("\\").isAbsolute());
                TLR_ASSERT(Path("C:").isAbsolute());
                TLR_ASSERT(Path("C:\\tmp").isAbsolute());
                TLR_ASSERT(!Path("").isAbsolute());
                TLR_ASSERT(!Path("../..").isAbsolute());
                TLR_ASSERT(!Path("..\\..").isAbsolute());
            }
        }
    }
}
