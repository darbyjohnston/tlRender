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
            _enums();
            _path();
            _util();
        }

        void PathTest::_enums()
        {
            _enum<UserPath>("UserPath", getUserPathEnums);
            for (auto i : getUserPathEnums())
            {
                _print(string::Format("{0}: {1}").arg(getLabel(i)).arg(getUserPath(i)));
            }
        }

        void PathTest::_path()
        {
            {
                PathOptions a;
                const PathOptions b;
                TLRENDER_ASSERT(a == b);
                a.maxNumberDigits = 0;
                TLRENDER_ASSERT(a != b);
            }
            {
                const Path path;
                TLRENDER_ASSERT(path.isEmpty());
                TLRENDER_ASSERT(path.getDirectory().empty());
                TLRENDER_ASSERT(path.getBaseName().empty());
                TLRENDER_ASSERT(path.getNumber().empty());
                TLRENDER_ASSERT(path.getExtension().empty());
            }
            {
                Path path("/tmp/file.txt");
                TLRENDER_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp", "file.txt");
                TLRENDER_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp/", "file.txt");
                TLRENDER_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("\\tmp\\file.txt");
                TLRENDER_ASSERT(path.get() == "\\tmp\\file.txt");
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
                    { "f", "", "f", "", 0, "" },
                    { "file", "", "file", "", 0, "" },
                    { "file.txt", "", "file", "", 0, ".txt" },
                    { "/tmp/file.txt", "/tmp/", "file", "", 0, ".txt" },
                    { "/tmp/render.1.exr", "/tmp/", "render.", "1", 0, ".exr" },
                    { "/tmp/render.0001.exr", "/tmp/", "render.", "0001", 4, ".exr" },
                    { "/tmp/render0001.exr", "/tmp/", "render", "0001", 4, ".exr" },
                    { ".", "", ".", "", 0, "" },
                    { "..", "", "..", "", 0, "" },
                    { "/.", "/", ".", "", 0, "" },
                    { "./", "./", "", "", 0, "" },
                    { ".dotfile", "", ".dotfile", "", 0, "" },
                    { "/tmp/.dotfile", "/tmp/", ".dotfile", "", 0, "" },
                    { "/tmp/.dotdir/.dotfile", "/tmp/.dotdir/", ".dotfile", "", 0, "" },
                    { "0", "", "", "0", 0, "" },
                    { "0001", "", "", "0001", 4, "" },
                    { "/tmp/0001", "/tmp/", "", "0001", 4, "" },
                    { "/tmp/0001.exr", "/tmp/", "", "0001", 4, ".exr" },
                    { "0001.exr", "", "", "0001", 4, ".exr" },
                    { "1.exr", "", "", "1", 0, ".exr" },
                    { "C:", "C:", "", "", 0, "" },
                    { "C:/", "C:/", "", "", 0, "" },
                    { "C:/tmp/file.txt", "C:/tmp/", "file", "", 0, ".txt" }
                };
                for (const auto& i : data)
                {
                    const Path path(i.fileName);
                    TLRENDER_ASSERT(i.fileName == path.get());
                    TLRENDER_ASSERT(i.directory == path.getDirectory());
                    TLRENDER_ASSERT(i.baseName == path.getBaseName());
                    TLRENDER_ASSERT(i.number == path.getNumber());
                    TLRENDER_ASSERT(i.padding == path.getPadding());
                    TLRENDER_ASSERT(i.extension == path.getExtension());
                }
            }
            {
                Path p("render.0001.exr");
                const math::IntRange sequence(1, 100);
                p.setSequence(sequence);
                TLRENDER_ASSERT(sequence == p.getSequence());
                TLRENDER_ASSERT(p.isSequence());
                TLRENDER_ASSERT(p.sequence(Path("render.0101.exr")));
                TLRENDER_ASSERT(!p.sequence(Path("render.101.exr")));
                TLRENDER_ASSERT("0001-0100" == p.getSequenceString());
            }
            {
                Path path("render.00000.exr");
                TLRENDER_ASSERT(path.sequence(Path("render.10000.exr")));
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
                const Path a("/");
                Path b("/");
                TLRENDER_ASSERT(a == b);
                b = Path("/tmp");
                TLRENDER_ASSERT(a != b);
            }
            {
                Path a("/tmp/render.1.exr");
                a.setDirectory("/usr/tmp/");
                TLRENDER_ASSERT(a.get() == "/usr/tmp/render.1.exr");
                a.setBaseName("comp.");
                TLRENDER_ASSERT(a.get() == "/usr/tmp/comp.1.exr");
                a.setNumber("0010");
                TLRENDER_ASSERT(a.get() == "/usr/tmp/comp.0010.exr");
                TLRENDER_ASSERT(a.getPadding() == 4);
                TLRENDER_ASSERT(a.getSequence() == math::IntRange(10, 10));
                a.setExtension(".tif");
                TLRENDER_ASSERT(a.get() == "/usr/tmp/comp.0010.tif");
            }
        }

        void PathTest::_util()
        {
            {
                TLRENDER_ASSERT(isPathSeparator('/'));
                TLRENDER_ASSERT(isPathSeparator('\\'));
            }
            {
                std::string path = appendSeparator(std::string());
                TLRENDER_ASSERT(path.empty());
                path = appendSeparator(std::string("/"));
                TLRENDER_ASSERT("/" == path);
                TLRENDER_ASSERT("/tmp/" == appendSeparator(std::string("/tmp")));
                TLRENDER_ASSERT("/tmp/" == appendSeparator(std::string("/tmp/")));
            }
            {
                std::string path = getParent("/usr/tmp");
                TLRENDER_ASSERT("/usr" == path);
                path = getParent("/tmp");
                TLRENDER_ASSERT("/" == path);
                path = getParent("a/b");
                TLRENDER_ASSERT("a" == path);
            }
            {
                _print(string::Format("Drives: {0}").arg(string::join(getDrives(), " ")));
            }
        }
    }
}
