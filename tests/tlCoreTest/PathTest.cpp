// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/PathTest.h>

#include <tlCore/Path.h>

#include <dtk/core/Format.h>
#include <dtk/core/String.h>

#include <iostream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        PathTest::PathTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "core_tests::PathTest")
        {}

        std::shared_ptr<PathTest> PathTest::create(const std::shared_ptr<dtk::Context>& context)
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
                _print(dtk::Format("{0}: {1}").arg(getLabel(i)).arg(getUserPath(i)));
            }
        }

        void PathTest::_path()
        {
            {
                PathOptions a;
                const PathOptions b;
                DTK_ASSERT(a == b);
                a.maxNumberDigits = 0;
                DTK_ASSERT(a != b);
            }
            {
                const Path path;
                DTK_ASSERT(path.isEmpty());
                DTK_ASSERT(path.getDirectory().empty());
                DTK_ASSERT(path.getBaseName().empty());
                DTK_ASSERT(path.getNumber().empty());
                DTK_ASSERT(path.getExtension().empty());
            }
            {
                Path path("/tmp/file.txt");
                DTK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp", "file.txt");
                DTK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("/tmp/", "file.txt");
                DTK_ASSERT(path.get() == "/tmp/file.txt");
                path = Path("\\tmp\\file.txt");
                DTK_ASSERT(path.get() == "\\tmp\\file.txt");
            }
            {
                std::string s = Path("tmp/", "render.", "0001", 4, ".exr", "http://", "?user=foo;password=bar").get();
                DTK_ASSERT(s == "http://tmp/render.0001.exr?user=foo;password=bar");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2);
                DTK_ASSERT(s == "http://tmp/render.0002.exr");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2, PathType::Path);
                DTK_ASSERT(s == "tmp/render.0002.exr");
                s = Path("tmp/", "render.", "0001", 4, ".exr", "http://").get(2, PathType::FileName);
                DTK_ASSERT(s == "render.0002.exr");
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
                    DTK_ASSERT(i.input == s);
                    DTK_ASSERT(i.protocol == path.getProtocol());
                    DTK_ASSERT(i.directory == path.getDirectory());
                    DTK_ASSERT(i.baseName == path.getBaseName());
                    DTK_ASSERT(i.number == path.getNumber());
                    DTK_ASSERT(i.padding == path.getPadding());
                    DTK_ASSERT(i.extension == path.getExtension());
                    DTK_ASSERT(i.request == path.getRequest());
                }
            }
            {
                Path p("render.0001.exr");
                const math::IntRange sequence(1, 100);
                p.setSequence(sequence);
                DTK_ASSERT(sequence == p.getSequence());
                DTK_ASSERT(p.isSequence());
                DTK_ASSERT("0001-0100" == p.getSequenceString());
                DTK_ASSERT(p.sequence(Path("render.0101.exr")));
                DTK_ASSERT(!p.sequence(Path("render.101.exr")));
            }
            {
                Path p("render.0001.exr");
                const math::IntRange sequence(1, 9999);
                p.setSequence(sequence);
                DTK_ASSERT("0001-9999" == p.getSequenceString());
                DTK_ASSERT(p.sequence(Path("render.0001.exr")));
                DTK_ASSERT(p.sequence(Path("render.1000.exr")));
                //! \bug Handle frame numbers that exceed the zero padding.
                //DTK_ASSERT(p.sequence(Path("render.10000.exr")));
            }
            {
                Path p("render.1000.exr");
                const math::IntRange sequence(1, 9999);
                p.setSequence(sequence);
                DTK_ASSERT(p.sequence(Path("render.0001.exr")));
                DTK_ASSERT(p.sequence(Path("render.1000.exr")));
                //! \bug How should the padding be handled in this case?
                //DTK_ASSERT("0001-9999" == p.getSequenceString());
            }
            {
                Path path("render.00000.exr");
                DTK_ASSERT(path.sequence(Path("render.10000.exr")));
            }
            {
                DTK_ASSERT(Path("/").isAbsolute());
                DTK_ASSERT(Path("/tmp").isAbsolute());
                DTK_ASSERT(Path("\\").isAbsolute());
                DTK_ASSERT(Path("C:").isAbsolute());
                DTK_ASSERT(Path("C:\\tmp").isAbsolute());
                DTK_ASSERT(!Path("").isAbsolute());
                DTK_ASSERT(!Path("../..").isAbsolute());
                DTK_ASSERT(!Path("..\\..").isAbsolute());
            }
            {
                const Path a("/");
                Path b("/");
                DTK_ASSERT(a == b);
                b = Path("/tmp");
                DTK_ASSERT(a != b);
            }
            {
                Path a("/tmp/render.1.exr");
                a.setProtocol("file://");
                DTK_ASSERT("file://" == a.getProtocol());
                DTK_ASSERT("file:" == a.getProtocolName());
                DTK_ASSERT(a.get() == "file:///tmp/render.1.exr");
                a.setDirectory("/usr/tmp/");
                DTK_ASSERT("/usr/tmp/" == a.getDirectory());
                DTK_ASSERT(a.get() == "file:///usr/tmp/render.1.exr");
                a.setBaseName("comp.");
                DTK_ASSERT("comp." == a.getBaseName());
                DTK_ASSERT(a.get() == "file:///usr/tmp/comp.1.exr");
                a.setNumber("0010");
                DTK_ASSERT("0010" == a.getNumber());
                DTK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.exr");
                DTK_ASSERT(a.getPadding() == 4);
                DTK_ASSERT(a.getSequence() == math::IntRange(10, 10));
                a.setExtension(".tif");
                DTK_ASSERT(".tif" == a.getExtension());
                DTK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.tif");
                a.setRequest("?user=foo;password=bar");
                DTK_ASSERT("?user=foo;password=bar" == a.getRequest());
                DTK_ASSERT(a.get() == "file:///usr/tmp/comp.0010.tif?user=foo;password=bar");
            }
        }

        void PathTest::_util()
        {
            {
                DTK_ASSERT(isPathSeparator('/'));
                DTK_ASSERT(isPathSeparator('\\'));
            }
            {
                std::string path = appendSeparator(std::string());
                DTK_ASSERT(path.empty());
                path = appendSeparator(std::string("/"));
                DTK_ASSERT("/" == path);
                DTK_ASSERT("/tmp/" == appendSeparator(std::string("/tmp")));
                DTK_ASSERT("/tmp/" == appendSeparator(std::string("/tmp/")));
            }
            {
                std::string path = getParent("/usr/tmp");
                DTK_ASSERT("/usr" == path);
                path = getParent("/tmp");
                DTK_ASSERT("/" == path);
                path = getParent("a/b");
                DTK_ASSERT("a" == path);
            }
            {
                _print(dtk::Format("Drives: {0}").arg(dtk::join(getDrives(), " ")));
            }
        }
    }
}
