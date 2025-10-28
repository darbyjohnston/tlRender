// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/PathTest.h>

#include <tlCore/Path.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <iostream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        PathTest::PathTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "core_tests::PathTest")
        {}

        std::shared_ptr<PathTest> PathTest::create(const std::shared_ptr<ftk::Context>& context)
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
                _print(ftk::Format("{0}: {1}").arg(getLabel(i)).arg(getUserPath(i)));
            }
        }

        void PathTest::_path()
        {
            {
                PathOptions a;
                const PathOptions b;
                FTK_ASSERT(a == b);
                a.maxNumberDigits = 0;
                FTK_ASSERT(a != b);
            }
            {
                const Path path;
                FTK_ASSERT(path.isEmpty());
                FTK_ASSERT(path.getDirectory().empty());
                FTK_ASSERT(path.getBaseName().empty());
                FTK_ASSERT(path.getNumber().empty());
                FTK_ASSERT(path.getExtension().empty());
            }
            {
                Path path("/tmp/file.txt");
                FTK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp", "file.txt");
                FTK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp/", "file.txt");
                FTK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("\\tmp\\file.txt");
                FTK_ASSERT(path.get() == "\\tmp\\file.txt");
            }
            {
                std::string s = Path("tmp/", "render.", "0001", 4, ".exr", "http://", "?user=foo;password=bar").get();
                FTK_ASSERT(s == "http://tmp/render.0001.exr?user=foo;password=bar");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2);
                FTK_ASSERT(s == "http://tmp/render.0002.exr");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2, PathType::Path);
                FTK_ASSERT(s == "tmp/render.0002.exr");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2, PathType::FileName);
                FTK_ASSERT(s == "render.0002.exr");
            }
            {
                struct Data
                {
                    std::string input;
                    std::string protocol;
                    std::string directory;
                    std::string baseName;
                    std::string number;
                    int padding = 0;
                    std::string extension;
                    std::string request;
                };
                const std::vector<Data> data =
                {
                    { "", "", "", "", "", 0, "", "" },
                    { "f", "", "", "f", "", 0, "", "" },
                    { "file", "", "", "file", "", 0, "", "" },
                    { "file.txt", "", "", "file", "", 0, ".txt", "" },
                    { "/tmp/file.txt", "", "/tmp/", "file", "", 0, ".txt", "" },
                    { "/tmp/render.1.exr", "", "/tmp/", "render.", "1", 0, ".exr", "" },
                    { "/tmp/render.0001.exr", "", "/tmp/", "render.", "0001", 4, ".exr", "" },
                    { "/tmp/render0001.exr", "", "/tmp/", "render", "0001", 4, ".exr", "" },
                    { ".", "", "", ".", "", 0, "", "" },
                    { "..", "", "", "..", "", 0, "", "" },
                    { "/.", "", "/", ".", "", 0, "", "" },
                    { "./", "", "./", "", "", 0, "", "" },
                    { ".dotfile", "", "", ".dotfile", "", 0, "", "" },
                    { "/tmp/.dotfile", "", "/tmp/", ".dotfile", "", 0, "", "" },
                    { "/tmp/.dotdir/.dotfile", "", "/tmp/.dotdir/", ".dotfile", "", 0, "", "" },
                    { "0", "", "", "", "0", 0, "", "" },
                    { "0001", "", "", "", "0001", 4, "", "" },
                    { "/tmp/0001", "", "/tmp/", "", "0001", 4, "", "" },
                    { "/tmp/0001.exr", "", "/tmp/", "", "0001", 4, ".exr", "" },
                    { "0001.exr", "", "", "", "0001", 4, ".exr", "" },
                    { "1.exr", "", "", "", "1", 0, ".exr", "" },
                    { "C:", "", "C:", "", "", 0, "", "" },
                    { "C:/", "", "C:/", "", "", 0, "", "" },
                    { "C:/tmp/file.txt", "", "C:/tmp/", "file", "", 0, ".txt", "" },
                    { "file:/tmp/render.1.exr", "file:", "/tmp/", "render.", "1", 0, ".exr", "" },
                    { "file://tmp/render.1.exr", "file://", "tmp/", "render.", "1", 0, ".exr", "" },
                    { "file:///tmp/render.1.exr", "file://", "/tmp/", "render.", "1", 0, ".exr", "" },
                    { "http://tmp/render.1.exr", "http://", "tmp/", "render.", "1", 0, ".exr", "" },
                    { "http://tmp/render.1.exr?user=foo;password=bar", "http://", "tmp/", "render.", "1", 0, ".exr", "?user=foo;password=bar" }
                };
                for (const auto& i : data)
                {
                    const Path path(i.input);
                    std::string s = path.get();
                    FTK_ASSERT(i.input == s);
                    FTK_ASSERT(i.protocol == path.getProtocol());
                    FTK_ASSERT(i.directory == path.getDirectory());
                    FTK_ASSERT(i.baseName == path.getBaseName());
                    FTK_ASSERT(i.number == path.getNumber());
                    FTK_ASSERT(i.padding == path.getPadding());
                    FTK_ASSERT(i.extension == path.getExtension());
                    FTK_ASSERT(i.request == path.getRequest());
                }
            }
            {
                Path p("render.0001.exr");
                const ftk::RangeI sequence(1, 100);
                p.setSequence(sequence);
                FTK_ASSERT(sequence == p.getSequence());
                FTK_ASSERT(p.isSequence());
                FTK_ASSERT("0001-0100" == p.getSequenceString());
                FTK_ASSERT(p.sequence(Path("render.0101.exr")));
                FTK_ASSERT(!p.sequence(Path("render.101.exr")));
            }
            {
                Path p("render.0001.exr");
                const ftk::RangeI sequence(1, 9999);
                p.setSequence(sequence);
                FTK_ASSERT("0001-9999" == p.getSequenceString());
                FTK_ASSERT(p.sequence(Path("render.0001.exr")));
                FTK_ASSERT(p.sequence(Path("render.1000.exr")));
                //! \bug Handle frame numbers that exceed the zero padding.
                //FTK_ASSERT(p.sequence(Path("render.10000.exr")));
            }
            {
                Path p("render.1000.exr");
                const ftk::RangeI sequence(1, 9999);
                p.setSequence(sequence);
                FTK_ASSERT(p.sequence(Path("render.0001.exr")));
                FTK_ASSERT(p.sequence(Path("render.1000.exr")));
                //! \bug How should the padding be handled in this case?
                //FTK_ASSERT("0001-9999" == p.getSequenceString());
            }
            {
                Path path("render.00000.exr");
                FTK_ASSERT(path.sequence(Path("render.10000.exr")));
            }
            {
                FTK_ASSERT(Path("/").isAbsolute());
                FTK_ASSERT(Path("/tmp").isAbsolute());
                FTK_ASSERT(Path("\\").isAbsolute());
                FTK_ASSERT(Path("C:").isAbsolute());
                FTK_ASSERT(Path("C:\\tmp").isAbsolute());
                FTK_ASSERT(!Path("").isAbsolute());
                FTK_ASSERT(!Path("../..").isAbsolute());
                FTK_ASSERT(!Path("..\\..").isAbsolute());
            }
            {
                const Path a("/");
                Path b("/");
                FTK_ASSERT(a == b);
                b = Path("/tmp");
                FTK_ASSERT(a != b);
            }
            {
                Path a("/tmp/render.1.exr");
                a.setProtocol("file://");
                FTK_ASSERT("file://" == a.getProtocol());
                FTK_ASSERT("file:" == a.getProtocolName());
                FTK_ASSERT(a.get() == "file:///tmp/render.1.exr");
                a.setDirectory("/usr/tmp/");
                FTK_ASSERT("/usr/tmp/" == a.getDirectory());
                FTK_ASSERT(a.get() == "file:///usr/tmp/render.1.exr");
                a.setBaseName("comp.");
                FTK_ASSERT("comp." == a.getBaseName());
                FTK_ASSERT(a.get() == "file:///usr/tmp/comp.1.exr");
                a.setNumber("0010");
                FTK_ASSERT("0010" == a.getNumber());
                FTK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.exr");
                FTK_ASSERT(a.getPadding() == 4);
                FTK_ASSERT(a.getSequence() == ftk::RangeI(10, 10));
                a.setExtension(".tif");
                FTK_ASSERT(".tif" == a.getExtension());
                FTK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.tif");
                a.setRequest("?user=foo;password=bar");
                FTK_ASSERT("?user=foo;password=bar" == a.getRequest());
                FTK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.tif?user=foo;password=bar");
            }
        }

        void PathTest::_util()
        {
            {
                FTK_ASSERT(isPathSeparator('/'));
                FTK_ASSERT(isPathSeparator('\\'));
            }
            {
                std::string path = appendSeparator(std::string());
                FTK_ASSERT(path.empty());
                path = appendSeparator(std::string("/"));
                FTK_ASSERT("/" == path);
                FTK_ASSERT("/tmp/" == appendSeparator(std::string("/tmp")));
                FTK_ASSERT("/tmp/" == appendSeparator(std::string("/tmp/")));
            }
            {
                std::string path = getParent("/usr/tmp");
                FTK_ASSERT("/usr" == path);
                path = getParent("/tmp");
                FTK_ASSERT("/" == path);
                path = getParent("a/b");
                FTK_ASSERT("a" == path);
            }
            {
                _print(ftk::Format("Drives: {0}").arg(ftk::join(getDrives(), " ")));
            }
        }
    }
}
