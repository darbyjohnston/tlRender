// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/PathTest.h>

#include <tlCore/Path.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

#include <iostream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        PathTest::PathTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "core_tests::PathTest")
        {}

        std::shared_ptr<PathTest> PathTest::create(const std::shared_ptr<feather_tk::Context>& context)
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
                _print(feather_tk::Format("{0}: {1}").arg(getLabel(i)).arg(getUserPath(i)));
            }
        }

        void PathTest::_path()
        {
            {
                PathOptions a;
                const PathOptions b;
                FEATHER_TK_ASSERT(a == b);
                a.maxNumberDigits = 0;
                FEATHER_TK_ASSERT(a != b);
            }
            {
                const Path path;
                FEATHER_TK_ASSERT(path.isEmpty());
                FEATHER_TK_ASSERT(path.getDirectory().empty());
                FEATHER_TK_ASSERT(path.getBaseName().empty());
                FEATHER_TK_ASSERT(path.getNumber().empty());
                FEATHER_TK_ASSERT(path.getExtension().empty());
            }
            {
                Path path("/tmp/file.txt");
                FEATHER_TK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp", "file.txt");
                FEATHER_TK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp/", "file.txt");
                FEATHER_TK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("\\tmp\\file.txt");
                FEATHER_TK_ASSERT(path.get() == "\\tmp\\file.txt");
            }
            {
                std::string s = Path("tmp/", "render.", "0001", 4, ".exr", "http://", "?user=foo;password=bar").get();
                FEATHER_TK_ASSERT(s == "http://tmp/render.0001.exr?user=foo;password=bar");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2);
                FEATHER_TK_ASSERT(s == "http://tmp/render.0002.exr");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2, PathType::Path);
                FEATHER_TK_ASSERT(s == "tmp/render.0002.exr");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2, PathType::FileName);
                FEATHER_TK_ASSERT(s == "render.0002.exr");
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
                    FEATHER_TK_ASSERT(i.input == s);
                    FEATHER_TK_ASSERT(i.protocol == path.getProtocol());
                    FEATHER_TK_ASSERT(i.directory == path.getDirectory());
                    FEATHER_TK_ASSERT(i.baseName == path.getBaseName());
                    FEATHER_TK_ASSERT(i.number == path.getNumber());
                    FEATHER_TK_ASSERT(i.padding == path.getPadding());
                    FEATHER_TK_ASSERT(i.extension == path.getExtension());
                    FEATHER_TK_ASSERT(i.request == path.getRequest());
                }
            }
            {
                Path p("render.0001.exr");
                const feather_tk::RangeI sequence(1, 100);
                p.setSequence(sequence);
                FEATHER_TK_ASSERT(sequence == p.getSequence());
                FEATHER_TK_ASSERT(p.isSequence());
                FEATHER_TK_ASSERT("0001-0100" == p.getSequenceString());
                FEATHER_TK_ASSERT(p.sequence(Path("render.0101.exr")));
                FEATHER_TK_ASSERT(!p.sequence(Path("render.101.exr")));
            }
            {
                Path p("render.0001.exr");
                const feather_tk::RangeI sequence(1, 9999);
                p.setSequence(sequence);
                FEATHER_TK_ASSERT("0001-9999" == p.getSequenceString());
                FEATHER_TK_ASSERT(p.sequence(Path("render.0001.exr")));
                FEATHER_TK_ASSERT(p.sequence(Path("render.1000.exr")));
                //! \bug Handle frame numbers that exceed the zero padding.
                //FEATHER_TK_ASSERT(p.sequence(Path("render.10000.exr")));
            }
            {
                Path p("render.1000.exr");
                const feather_tk::RangeI sequence(1, 9999);
                p.setSequence(sequence);
                FEATHER_TK_ASSERT(p.sequence(Path("render.0001.exr")));
                FEATHER_TK_ASSERT(p.sequence(Path("render.1000.exr")));
                //! \bug How should the padding be handled in this case?
                //FEATHER_TK_ASSERT("0001-9999" == p.getSequenceString());
            }
            {
                Path path("render.00000.exr");
                FEATHER_TK_ASSERT(path.sequence(Path("render.10000.exr")));
            }
            {
                FEATHER_TK_ASSERT(Path("/").isAbsolute());
                FEATHER_TK_ASSERT(Path("/tmp").isAbsolute());
                FEATHER_TK_ASSERT(Path("\\").isAbsolute());
                FEATHER_TK_ASSERT(Path("C:").isAbsolute());
                FEATHER_TK_ASSERT(Path("C:\\tmp").isAbsolute());
                FEATHER_TK_ASSERT(!Path("").isAbsolute());
                FEATHER_TK_ASSERT(!Path("../..").isAbsolute());
                FEATHER_TK_ASSERT(!Path("..\\..").isAbsolute());
            }
            {
                const Path a("/");
                Path b("/");
                FEATHER_TK_ASSERT(a == b);
                b = Path("/tmp");
                FEATHER_TK_ASSERT(a != b);
            }
            {
                Path a("/tmp/render.1.exr");
                a.setProtocol("file://");
                FEATHER_TK_ASSERT("file://" == a.getProtocol());
                FEATHER_TK_ASSERT("file:" == a.getProtocolName());
                FEATHER_TK_ASSERT(a.get() == "file:///tmp/render.1.exr");
                a.setDirectory("/usr/tmp/");
                FEATHER_TK_ASSERT("/usr/tmp/" == a.getDirectory());
                FEATHER_TK_ASSERT(a.get() == "file:///usr/tmp/render.1.exr");
                a.setBaseName("comp.");
                FEATHER_TK_ASSERT("comp." == a.getBaseName());
                FEATHER_TK_ASSERT(a.get() == "file:///usr/tmp/comp.1.exr");
                a.setNumber("0010");
                FEATHER_TK_ASSERT("0010" == a.getNumber());
                FEATHER_TK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.exr");
                FEATHER_TK_ASSERT(a.getPadding() == 4);
                FEATHER_TK_ASSERT(a.getSequence() == feather_tk::RangeI(10, 10));
                a.setExtension(".tif");
                FEATHER_TK_ASSERT(".tif" == a.getExtension());
                FEATHER_TK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.tif");
                a.setRequest("?user=foo;password=bar");
                FEATHER_TK_ASSERT("?user=foo;password=bar" == a.getRequest());
                FEATHER_TK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.tif?user=foo;password=bar");
            }
        }

        void PathTest::_util()
        {
            {
                FEATHER_TK_ASSERT(isPathSeparator('/'));
                FEATHER_TK_ASSERT(isPathSeparator('\\'));
            }
            {
                std::string path = appendSeparator(std::string());
                FEATHER_TK_ASSERT(path.empty());
                path = appendSeparator(std::string("/"));
                FEATHER_TK_ASSERT("/" == path);
                FEATHER_TK_ASSERT("/tmp/" == appendSeparator(std::string("/tmp")));
                FEATHER_TK_ASSERT("/tmp/" == appendSeparator(std::string("/tmp/")));
            }
            {
                std::string path = getParent("/usr/tmp");
                FEATHER_TK_ASSERT("/usr" == path);
                path = getParent("/tmp");
                FEATHER_TK_ASSERT("/" == path);
                path = getParent("a/b");
                FEATHER_TK_ASSERT("a" == path);
            }
            {
                _print(feather_tk::Format("Drives: {0}").arg(feather_tk::join(getDrives(), " ")));
            }
        }
    }
}
