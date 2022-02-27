// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#if defined(TLRENDER_BUILD_QT5) || defined(TLRENDER_BUILD_QT6)
#include <tlQtTest/TimeObjectTest.h>
#endif

#include <tlTimelineTest/TimelinePlayerTest.h>
#include <tlTimelineTest/TimelineTest.h>
#include <tlTimelineTest/TimelineUtilTest.h>

#if defined(TLRENDER_BUILD_GL)
#include <tlRenderGLTest/MeshTest.h>
#endif

#include <tlIOTest/CineonTest.h>
#include <tlIOTest/DPXTest.h>
#include <tlIOTest/IOTest.h>
#include <tlIOTest/PPMTest.h>
#include <tlIOTest/SGITest.h>
#if defined(FFmpeg_FOUND)
#include <tlIOTest/FFmpegTest.h>
#endif
#if defined(JPEG_FOUND)
#include <tlIOTest/JPEGTest.h>
#endif
#if defined(OpenEXR_FOUND)
#include <tlIOTest/OpenEXRTest.h>
#endif
#if defined(PNG_FOUND)
#include <tlIOTest/PNGTest.h>
#endif
#if defined(TIFF_FOUND)
#include <tlIOTest/TIFFTest.h>
#endif

#include <tlCoreTest/AudioTest.h>
#include <tlCoreTest/BBoxTest.h>
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
#include <tlCoreTest/PathTest.h>
#include <tlCoreTest/RangeTest.h>
#include <tlCoreTest/StringTest.h>
#include <tlCoreTest/StringFormatTest.h>
#include <tlCoreTest/TimeTest.h>
#include <tlCoreTest/ValueObserverTest.h>

#include <tlIO/IOSystem.h>

#include <tlCore/Context.h>

#include <iostream>
#include <vector>

using namespace tl::core;
using namespace tl::tests;

int main(int argc, char* argv[])
{
    auto context = Context::create();
    context->addSystem(tl::io::System::create(context));
    
    for (const auto& i : context->getLogInit())
    {
        std::cout << "[LOG] " << toString(i) << std::endl;
    }
    auto logObserver = observer::ValueObserver<LogItem>::create(
        context->getSystem<LogSystem>()->observeLog(),
        [](const LogItem& value)
        {
            std::cout << "[LOG] " << toString(value) << std::endl;
        },
        observer::CallbackAction::Suppress);

    std::vector<std::shared_ptr<Test::ITest> > tests;
    if (0)
    {
    }
    else
    {
        tests.push_back(core_test::AudioTest::create(context));
        tests.push_back(core_test::BBoxTest::create(context));
        tests.push_back(core_test::ColorTest::create(context));
        tests.push_back(core_test::ContextTest::create(context));
        tests.push_back(core_test::ErrorTest::create(context));
        tests.push_back(core_test::FileTest::create(context));
        tests.push_back(core_test::FileIOTest::create(context));
        tests.push_back(core_test::ImageTest::create(context));
        tests.push_back(core_test::LRUCacheTest::create(context));
        tests.push_back(core_test::ListObserverTest::create(context));
        tests.push_back(core_test::MapObserverTest::create(context));
        tests.push_back(core_test::MathTest::create(context));
        tests.push_back(core_test::MemoryTest::create(context));
        tests.push_back(core_test::PathTest::create(context));
        tests.push_back(core_test::RangeTest::create(context));
        tests.push_back(core_test::StringTest::create(context));
        tests.push_back(core_test::StringFormatTest::create(context));
        tests.push_back(core_test::TimeTest::create(context));
        tests.push_back(core_test::ValueObserverTest::create(context));

        tests.push_back(io_test::CineonTest::create(context));
        tests.push_back(io_test::DPXTest::create(context));
        tests.push_back(io_test::IOTest::create(context));
        tests.push_back(io_test::PPMTest::create(context));
        tests.push_back(io_test::SGITest::create(context));
#if defined(FFmpeg_FOUND)
        tests.push_back(io_test::FFmpegTest::create(context));
#endif
#if defined(JPEG_FOUND)
        tests.push_back(io_test::JPEGTest::create(context));
#endif
#if defined(OpenEXR_FOUND)
        tests.push_back(io_test::OpenEXRTest::create(context));
#endif
#if defined(PNG_FOUND)
        tests.push_back(io_test::PNGTest::create(context));
#endif
#if defined(TIFF_FOUND)
        tests.push_back(io_test::TIFFTest::create(context));
#endif

        tests.push_back(timeline_test::TimelinePlayerTest::create(context));
        tests.push_back(timeline_test::TimelineTest::create(context));
        tests.push_back(timeline_test::TimelineUtilTest::create(context));

#if defined(TLRENDER_BUILD_GL)
        tests.push_back(render_gl_test::MeshTest::create(context));
#endif

#if defined(TLRENDER_BUILD_QT5) || defined(TLRENDER_BUILD_QT6)
        tests.push_back(qt_test::TimeObjectTest::create(context));
#endif
    }
    for (const auto& i : tests)
    {
        std::cout << "Running test: " << i->getName() << std::endl;
        i->run();
    }

    return 0;
}
