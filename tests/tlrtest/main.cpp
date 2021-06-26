// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/AVIOTest.h>
#include <tlrCoreTest/BBoxTest.h>
#include <tlrCoreTest/CacheTest.h>
#include <tlrCoreTest/ColorTest.h>
#include <tlrCoreTest/ErrorTest.h>
#include <tlrCoreTest/FileTest.h>
#include <tlrCoreTest/ImageTest.h>
#include <tlrCoreTest/ListObserverTest.h>
#include <tlrCoreTest/MapObserverTest.h>
#include <tlrCoreTest/MathTest.h>
#include <tlrCoreTest/MatrixTest.h>
#include <tlrCoreTest/MemoryTest.h>
#include <tlrCoreTest/RangeTest.h>
#include <tlrCoreTest/StringTest.h>
#include <tlrCoreTest/StringFormatTest.h>
#include <tlrCoreTest/TimeTest.h>
#include <tlrCoreTest/TimelinePlayerTest.h>
#include <tlrCoreTest/TimelineTest.h>
#include <tlrCoreTest/ValueObserverTest.h>
#include <tlrCoreTest/VectorTest.h>
#if defined(FFmpeg_FOUND)
#include <tlrCoreTest/FFmpegTest.h>
#endif
#if defined(JPEG_FOUND)
#include <tlrCoreTest/JPEGTest.h>
#endif
#if defined(OpenEXR_FOUND)
#include <tlrCoreTest/OpenEXRTest.h>
#endif
#if defined(PNG_FOUND)
#include <tlrCoreTest/PNGTest.h>
#endif
#if defined(TIFF_FOUND)
#include <tlrCoreTest/TIFFTest.h>
#endif

#if defined(TLR_BUILD_GL)
#include <tlrGLTest/MeshTest.h>
#endif

#if defined(TLR_BUILD_QT)
#include <tlrQtTest/TimeObjectTest.h>
#endif

#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    std::vector<std::shared_ptr<tlr::Test::ITest> > tests;

    if (0)
    {
        tests.push_back(tlr::CoreTest::FFmpegTest::create());
    }
    else
    {
        tests.push_back(tlr::CoreTest::AVIOTest::create());
        tests.push_back(tlr::CoreTest::BBoxTest::create());
        tests.push_back(tlr::CoreTest::CacheTest::create());
        tests.push_back(tlr::CoreTest::ColorTest::create());
        tests.push_back(tlr::CoreTest::ErrorTest::create());
        tests.push_back(tlr::CoreTest::FileTest::create());
        tests.push_back(tlr::CoreTest::ImageTest::create());
        tests.push_back(tlr::CoreTest::ListObserverTest::create());
        tests.push_back(tlr::CoreTest::MapObserverTest::create());
        tests.push_back(tlr::CoreTest::MathTest::create());
        tests.push_back(tlr::CoreTest::MatrixTest::create());
        tests.push_back(tlr::CoreTest::MemoryTest::create());
        tests.push_back(tlr::CoreTest::RangeTest::create());
        tests.push_back(tlr::CoreTest::StringTest::create());
        tests.push_back(tlr::CoreTest::StringFormatTest::create());
        tests.push_back(tlr::CoreTest::TimeTest::create());
        tests.push_back(tlr::CoreTest::TimelinePlayerTest::create());
        tests.push_back(tlr::CoreTest::TimelineTest::create());
        tests.push_back(tlr::CoreTest::ValueObserverTest::create());
        tests.push_back(tlr::CoreTest::VectorTest::create());
#if defined(FFmpeg_FOUND)
        tests.push_back(tlr::CoreTest::FFmpegTest::create());
#endif
#if defined(JPEG_FOUND)
        tests.push_back(tlr::CoreTest::JPEGTest::create());
#endif
#if defined(OpenEXR_FOUND)
        tests.push_back(tlr::CoreTest::OpenEXRTest::create());
#endif
#if defined(PNG_FOUND)
        tests.push_back(tlr::CoreTest::PNGTest::create());
#endif
#if defined(TIFF_FOUND)
        tests.push_back(tlr::CoreTest::TIFFTest::create());
#endif

#if defined(TLR_BUILD_GL)
        tests.push_back(tlr::GLTest::MeshTest::create());
#endif

#if defined(TLR_BUILD_QT)
        tests.push_back(tlr::QtTest::TimeObjectTest::create());
#endif
    }

    for (const auto& i : tests)
    {
        std::cout << "Running test: " << i->getName() << std::endl;
        i->run();
    }

    return 0;
}
