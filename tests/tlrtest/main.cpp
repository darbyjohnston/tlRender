// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/AVIOTest.h>
#include <tlrCoreTest/BBoxTest.h>
#include <tlrCoreTest/CineonTest.h>
#include <tlrCoreTest/DPXTest.h>
#include <tlrCoreTest/ColorTest.h>
#include <tlrCoreTest/ContextTest.h>
#include <tlrCoreTest/ErrorTest.h>
#include <tlrCoreTest/FileTest.h>
#include <tlrCoreTest/FileIOTest.h>
#include <tlrCoreTest/ImageTest.h>
#include <tlrCoreTest/LRUCacheTest.h>
#include <tlrCoreTest/ListObserverTest.h>
#include <tlrCoreTest/MapObserverTest.h>
#include <tlrCoreTest/MathTest.h>
#include <tlrCoreTest/MatrixTest.h>
#include <tlrCoreTest/MemoryTest.h>
#include <tlrCoreTest/PPMTest.h>
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

using namespace tlr;

int main(int argc, char* argv[])
{
    auto context = core::Context::create();
    
    for (const auto& i : context->getLogInit())
    {
        std::cout << "[LOG] " << core::toString(i) << std::endl;
    }
    auto logObserver = observer::ValueObserver<core::LogItem>::create(
        context->getSystem<core::LogSystem>()->observeLog(),
        [](const core::LogItem& value)
        {
            std::cout << "[LOG] " << core::toString(value) << std::endl;
        });

    std::vector<std::shared_ptr<Test::ITest> > tests;
    if (0)
    {
        tests.push_back(CoreTest::FileIOTest::create(context));
    }
    else
    {
        tests.push_back(CoreTest::AVIOTest::create(context));
        tests.push_back(CoreTest::BBoxTest::create(context));
        tests.push_back(CoreTest::CineonTest::create(context));
        tests.push_back(CoreTest::DPXTest::create(context));
        tests.push_back(CoreTest::ColorTest::create(context));
        tests.push_back(CoreTest::ContextTest::create(context));
        tests.push_back(CoreTest::ErrorTest::create(context));
        tests.push_back(CoreTest::FileTest::create(context));
        tests.push_back(CoreTest::FileIOTest::create(context));
        tests.push_back(CoreTest::ImageTest::create(context));
        tests.push_back(CoreTest::LRUCacheTest::create(context));
        tests.push_back(CoreTest::ListObserverTest::create(context));
        tests.push_back(CoreTest::MapObserverTest::create(context));
        tests.push_back(CoreTest::MathTest::create(context));
        tests.push_back(CoreTest::MatrixTest::create(context));
        tests.push_back(CoreTest::MemoryTest::create(context));
        tests.push_back(CoreTest::PPMTest::create(context));
        tests.push_back(CoreTest::PathTest::create(context));
        tests.push_back(CoreTest::RangeTest::create(context));
        tests.push_back(CoreTest::StringTest::create(context));
        tests.push_back(CoreTest::StringFormatTest::create(context));
        tests.push_back(CoreTest::TimeTest::create(context));
        tests.push_back(CoreTest::TimelinePlayerTest::create(context));
        tests.push_back(CoreTest::TimelineTest::create(context));
        tests.push_back(CoreTest::ValueObserverTest::create(context));
        tests.push_back(CoreTest::VectorTest::create(context));
#if defined(FFmpeg_FOUND)
        tests.push_back(CoreTest::FFmpegTest::create(context));
#endif
#if defined(JPEG_FOUND)
        tests.push_back(CoreTest::JPEGTest::create(context));
#endif
#if defined(OpenEXR_FOUND)
        tests.push_back(CoreTest::OpenEXRTest::create(context));
#endif
#if defined(PNG_FOUND)
        tests.push_back(CoreTest::PNGTest::create(context));
#endif
#if defined(TIFF_FOUND)
        tests.push_back(CoreTest::TIFFTest::create(context));
#endif

#if defined(TLR_BUILD_GL)
        tests.push_back(GLTest::MeshTest::create(context));
#endif

#if defined(TLR_BUILD_QT)
        tests.push_back(QtTest::TimeObjectTest::create(context));
#endif
    }
    for (const auto& i : tests)
    {
        std::cout << "Running test: " << i->getName() << std::endl;
        i->run();
    }

    return 0;
}
