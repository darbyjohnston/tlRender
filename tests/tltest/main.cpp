// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/AVIOTest.h>
#include <tlCoreTest/AudioTest.h>
#include <tlCoreTest/BBoxTest.h>
#include <tlCoreTest/CineonTest.h>
#include <tlCoreTest/DPXTest.h>
#include <tlCoreTest/ColorTest.h>
#include <tlCoreTest/ContextTest.h>
#include <tlCoreTest/ErrorTest.h>
#include <tlCoreTest/FileTest.h>
#include <tlCoreTest/FileIOTest.h>
#include <tlCoreTest/ImageTest.h>
#include <tlCoreTest/LRUCacheTest.h>
#include <tlCoreTest/ListObserverTest.h>
#include <tlCoreTest/MapObserverTest.h>
#include <tlCoreTest/MathTest.h>
#include <tlCoreTest/MemoryTest.h>
#include <tlCoreTest/PPMTest.h>
#include <tlCoreTest/PathTest.h>
#include <tlCoreTest/RangeTest.h>
#include <tlCoreTest/SGITest.h>
#include <tlCoreTest/StringTest.h>
#include <tlCoreTest/StringFormatTest.h>
#include <tlCoreTest/TimeTest.h>
#include <tlCoreTest/TimelinePlayerTest.h>
#include <tlCoreTest/TimelineTest.h>
#include <tlCoreTest/TimelineUtilTest.h>
#include <tlCoreTest/ValueObserverTest.h>
#if defined(FFmpeg_FOUND)
#include <tlCoreTest/FFmpegTest.h>
#endif
#if defined(JPEG_FOUND)
#include <tlCoreTest/JPEGTest.h>
#endif
#if defined(OpenEXR_FOUND)
#include <tlCoreTest/OpenEXRTest.h>
#endif
#if defined(PNG_FOUND)
#include <tlCoreTest/PNGTest.h>
#endif
#if defined(TIFF_FOUND)
#include <tlCoreTest/TIFFTest.h>
#endif

#if defined(TLRENDER_BUILD_GL)
#include <tlGLTest/MeshTest.h>
#endif

#if defined(TLRENDER_BUILD_QT)
#include <tlQtTest/TimeObjectTest.h>
#endif

#include <tlCore/Context.h>

#include <iostream>
#include <vector>

using namespace tl;

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
        },
        observer::CallbackAction::Suppress);

    std::vector<std::shared_ptr<Test::ITest> > tests;
    if (0)
    {
        tests.push_back(CoreTest::SGITest::create(context));
    }
    else
    {
        tests.push_back(CoreTest::AVIOTest::create(context));
        tests.push_back(CoreTest::AudioTest::create(context));
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
        tests.push_back(CoreTest::MemoryTest::create(context));
        tests.push_back(CoreTest::PPMTest::create(context));
        tests.push_back(CoreTest::PathTest::create(context));
        tests.push_back(CoreTest::RangeTest::create(context));
        tests.push_back(CoreTest::SGITest::create(context));
        tests.push_back(CoreTest::StringTest::create(context));
        tests.push_back(CoreTest::StringFormatTest::create(context));
        tests.push_back(CoreTest::TimeTest::create(context));
        tests.push_back(CoreTest::TimelinePlayerTest::create(context));
        tests.push_back(CoreTest::TimelineTest::create(context));
        tests.push_back(CoreTest::TimelineUtilTest::create(context));
        tests.push_back(CoreTest::ValueObserverTest::create(context));
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

#if defined(TLRENDER_BUILD_GL)
        tests.push_back(GLTest::MeshTest::create(context));
#endif

#if defined(TLRENDER_BUILD_QT)
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
