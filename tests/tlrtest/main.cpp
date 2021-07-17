// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/AVIOTest.h>
#include <tlrCoreTest/BBoxTest.h>
#include <tlrCoreTest/CineonTest.h>
#include <tlrCoreTest/DPXTest.h>
#include <tlrCoreTest/ColorTest.h>
#include <tlrCoreTest/ErrorTest.h>
#include <tlrCoreTest/FileTest.h>
#include <tlrCoreTest/ImageTest.h>
#include <tlrCoreTest/LRUCacheTest.h>
#include <tlrCoreTest/ListObserverTest.h>
#include <tlrCoreTest/MapObserverTest.h>
#include <tlrCoreTest/MathTest.h>
#include <tlrCoreTest/MatrixTest.h>
#include <tlrCoreTest/MemoryTest.h>
#include <tlrCoreTest/PathTest.h>
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

#include <tlrCore/Context.h>

#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    auto context = tlr::core::Context::create();

    for (const auto& i : context->getLogInit())
    {
        std::cout << i << std::endl;
    }
    auto logObserver = tlr::observer::ValueObserver<std::string>::create(
        context->getSystem<tlr::core::LogSystem>()->observeLog(),
        [](const std::string& value)
        {
            std::cout << value << std::endl;
        });

    std::vector<std::shared_ptr<tlr::Test::ITest> > tests;
    if (0)
    {
        tests.push_back(tlr::CoreTest::ImageTest::create(context));
    }
    else
    {
        tests.push_back(tlr::CoreTest::AVIOTest::create(context));
        tests.push_back(tlr::CoreTest::BBoxTest::create(context));
        tests.push_back(tlr::CoreTest::CineonTest::create(context));
        tests.push_back(tlr::CoreTest::DPXTest::create(context));
        tests.push_back(tlr::CoreTest::ColorTest::create(context));
        tests.push_back(tlr::CoreTest::ErrorTest::create(context));
        tests.push_back(tlr::CoreTest::FileTest::create(context));
        tests.push_back(tlr::CoreTest::ImageTest::create(context));
        tests.push_back(tlr::CoreTest::LRUCacheTest::create(context));
        tests.push_back(tlr::CoreTest::ListObserverTest::create(context));
        tests.push_back(tlr::CoreTest::MapObserverTest::create(context));
        tests.push_back(tlr::CoreTest::MathTest::create(context));
        tests.push_back(tlr::CoreTest::MatrixTest::create(context));
        tests.push_back(tlr::CoreTest::MemoryTest::create(context));
        tests.push_back(tlr::CoreTest::PathTest::create(context));
        tests.push_back(tlr::CoreTest::RangeTest::create(context));
        tests.push_back(tlr::CoreTest::StringTest::create(context));
        tests.push_back(tlr::CoreTest::StringFormatTest::create(context));
        tests.push_back(tlr::CoreTest::TimeTest::create(context));
        tests.push_back(tlr::CoreTest::TimelinePlayerTest::create(context));
        tests.push_back(tlr::CoreTest::TimelineTest::create(context));
        tests.push_back(tlr::CoreTest::ValueObserverTest::create(context));
        tests.push_back(tlr::CoreTest::VectorTest::create(context));
#if defined(FFmpeg_FOUND)
        tests.push_back(tlr::CoreTest::FFmpegTest::create(context));
#endif
#if defined(JPEG_FOUND)
        tests.push_back(tlr::CoreTest::JPEGTest::create(context));
#endif
#if defined(OpenEXR_FOUND)
        tests.push_back(tlr::CoreTest::OpenEXRTest::create(context));
#endif
#if defined(PNG_FOUND)
        tests.push_back(tlr::CoreTest::PNGTest::create(context));
#endif
#if defined(TIFF_FOUND)
        tests.push_back(tlr::CoreTest::TIFFTest::create(context));
#endif

#if defined(TLR_BUILD_GL)
        tests.push_back(tlr::GLTest::MeshTest::create(context));
#endif

#if defined(TLR_BUILD_QT)
        tests.push_back(tlr::QtTest::TimeObjectTest::create(context));
#endif
    }
    for (const auto& i : tests)
    {
        std::cout << "Running test: " << i->getName() << std::endl;
        i->run();
    }

    return 0;
}
