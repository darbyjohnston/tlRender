// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/BBoxTest.h>
#include <tlrCoreTest/CacheTest.h>
#include <tlrCoreTest/ColorTest.h>
#include <tlrCoreTest/FileTest.h>
#include <tlrCoreTest/IOTest.h>
#include <tlrCoreTest/ImageTest.h>
#include <tlrCoreTest/TimelineTest.h>
#if defined(FFmpeg_FOUND)
#include <tlrCoreTest/FFmpegTest.h>
#endif

#if defined(TLR_BUILD_GL)
#include <tlrGLTest/MeshTest.h>
#endif

#if defined(TLR_BUILD_QT)
#include <tlrQtTest/TimeObjectTest.h>
#endif

#include <vector>

int main(int argc, char* argv[])
{
    std::vector<std::shared_ptr<tlr::Test::ITest> > tests;

    tests.push_back(tlr::CoreTest::BBoxTest::create());
    tests.push_back(tlr::CoreTest::CacheTest::create());
    tests.push_back(tlr::CoreTest::ColorTest::create());
    tests.push_back(tlr::CoreTest::FileTest::create());
    tests.push_back(tlr::CoreTest::IOTest::create());
    tests.push_back(tlr::CoreTest::ImageTest::create());
    tests.push_back(tlr::CoreTest::TimelineTest::create());
#if defined(FFmpeg_FOUND)
    tests.push_back(tlr::CoreTest::FFmpegTest::create());
#endif

#if defined(TLR_BUILD_GL)
    tests.push_back(tlr::GLTest::MeshTest::create());
#endif

#if defined(TLR_BUILD_QT)
    tests.push_back(tlr::QtTest::TimeObjectTest::create());
#endif

    for (const auto& i : tests)
    {
        std::cout << "Running test: " << i->getName() << std::endl;
        i->run();
    }

    return 0;
}
