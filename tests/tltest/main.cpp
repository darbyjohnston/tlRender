// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
#include <tlQtTest/TimeObjectTest.h>
#include <tlQt/Util.h>
#endif // TLRENDER_QT5 || TLRENDER_QT6

#if defined(TLRENDER_GL)
#include <tlGLTest/MeshTest.h>
#include <tlGL/Util.h>
#endif // TLRENDER_GL

#include <tlAppTest/AppTest.h>
#include <tlAppTest/CmdLineTest.h>

#include <tlTimelineTest/ColorConfigOptionsTest.h>
#include <tlTimelineTest/IRenderTest.h>
#include <tlTimelineTest/LUTOptionsTest.h>
#include <tlTimelineTest/TimelinePlayerTest.h>
#include <tlTimelineTest/TimelineTest.h>
#include <tlTimelineTest/TimelineUtilTest.h>

#include <tlIOTest/CineonTest.h>
#include <tlIOTest/DPXTest.h>
#include <tlIOTest/IOTest.h>
#include <tlIOTest/PPMTest.h>
#include <tlIOTest/SGITest.h>
#if defined(TLRENDER_FFMPEG)
#include <tlIOTest/FFmpegTest.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
#include <tlIOTest/JPEGTest.h>
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
#include <tlIOTest/OpenEXRTest.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_PNG)
#include <tlIOTest/PNGTest.h>
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
#include <tlIOTest/TIFFTest.h>
#endif // TLRENDER_TIFF
#include <tlIO/Util.h>

#include <tlCoreTest/AudioTest.h>
#include <tlCoreTest/BBoxTest.h>
#include <tlCoreTest/ColorTest.h>
#include <tlCoreTest/ContextTest.h>
#include <tlCoreTest/ErrorTest.h>
#include <tlCoreTest/FileIOTest.h>
#include <tlCoreTest/FileInfoTest.h>
#include <tlCoreTest/FileTest.h>
#include <tlCoreTest/FontSystemTest.h>
#include <tlCoreTest/HDRTest.h>
#include <tlCoreTest/ImageTest.h>
#include <tlCoreTest/LRUCacheTest.h>
#include <tlCoreTest/ListObserverTest.h>
#include <tlCoreTest/MapObserverTest.h>
#include <tlCoreTest/MathTest.h>
#include <tlCoreTest/MatrixTest.h>
#include <tlCoreTest/MemoryTest.h>
#include <tlCoreTest/MeshTest.h>
#include <tlCoreTest/OSTest.h>
#include <tlCoreTest/PathTest.h>
#include <tlCoreTest/RangeTest.h>
#include <tlCoreTest/StringTest.h>
#include <tlCoreTest/StringFormatTest.h>
#include <tlCoreTest/TimeTest.h>
#include <tlCoreTest/ValueObserverTest.h>
#include <tlCoreTest/VectorTest.h>

#include <tlCore/Context.h>

#include <iostream>
#include <vector>

using namespace tl;
using namespace tl::tests;

int main(int argc, char* argv[])
{
    auto context = system::Context::create();
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
    qt::init(context);
#elif defined(TLRENDER_GL)
    gl::init(context);
#else // TLRENDER_QT5 || TLRENDER_QT6
    io::init(context);
#endif // TLRENDER_QT5 || TLRENDER_QT6

    auto logObserver = observer::ListObserver<log::Item>::create(
        context->getSystem<log::System>()->observeLog(),
        [](const std::vector<log::Item>& value)
        {
            for (const auto& i : value)
            {
                std::cout << "[LOG] " << toString(i) << std::endl;
            }
        },
        observer::CallbackAction::Suppress);

    context->tick();

    std::vector<std::shared_ptr<tests::ITest> > tests;
    if (0)
    {
        tests.push_back(core_tests::TimeTest::create(context));
    }
    else
    {
        if (1)
        {
            tests.push_back(core_tests::AudioTest::create(context));
            tests.push_back(core_tests::BBoxTest::create(context));
            tests.push_back(core_tests::ColorTest::create(context));
            tests.push_back(core_tests::ContextTest::create(context));
            tests.push_back(core_tests::ErrorTest::create(context));
            tests.push_back(core_tests::FileIOTest::create(context));
            tests.push_back(core_tests::FileInfoTest::create(context));
            tests.push_back(core_tests::FileTest::create(context));
            tests.push_back(core_tests::FontSystemTest::create(context));
            tests.push_back(core_tests::HDRTest::create(context));
            tests.push_back(core_tests::ImageTest::create(context));
            tests.push_back(core_tests::LRUCacheTest::create(context));
            tests.push_back(core_tests::ListObserverTest::create(context));
            tests.push_back(core_tests::MapObserverTest::create(context));
            tests.push_back(core_tests::MathTest::create(context));
            tests.push_back(core_tests::MatrixTest::create(context));
            tests.push_back(core_tests::MemoryTest::create(context));
            tests.push_back(core_tests::MeshTest::create(context));
            tests.push_back(core_tests::OSTest::create(context));
            tests.push_back(core_tests::PathTest::create(context));
            tests.push_back(core_tests::RangeTest::create(context));
            tests.push_back(core_tests::StringTest::create(context));
            tests.push_back(core_tests::StringFormatTest::create(context));
            tests.push_back(core_tests::TimeTest::create(context));
            tests.push_back(core_tests::ValueObserverTest::create(context));
            tests.push_back(core_tests::VectorTest::create(context));
        }
        if (1)
        {
            tests.push_back(io_tests::CineonTest::create(context));
            tests.push_back(io_tests::DPXTest::create(context));
            tests.push_back(io_tests::IOTest::create(context));
            tests.push_back(io_tests::PPMTest::create(context));
            tests.push_back(io_tests::SGITest::create(context));
#if defined(TLRENDER_FFMPEG)
            tests.push_back(io_tests::FFmpegTest::create(context));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
            tests.push_back(io_tests::JPEGTest::create(context));
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
            tests.push_back(io_tests::OpenEXRTest::create(context));
#endif // TLRENDER_EXR
#if defined(TLRENDER_PNG)
            tests.push_back(io_tests::PNGTest::create(context));
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
            tests.push_back(io_tests::TIFFTest::create(context));
#endif // TLRENDER_TIFF
        }
        if (1)
        {
            tests.push_back(timeline_tests::ColorConfigOptionsTest::create(context));
            tests.push_back(timeline_tests::IRenderTest::create(context));
            tests.push_back(timeline_tests::LUTOptionsTest::create(context));
            tests.push_back(timeline_tests::TimelinePlayerTest::create(context));
            tests.push_back(timeline_tests::TimelineTest::create(context));
            tests.push_back(timeline_tests::TimelineUtilTest::create(context));
        }
        if (1)
        {
            tests.push_back(app_tests::AppTest::create(context));
            tests.push_back(app_tests::CmdLineTest::create(context));
        }
        if (1)
        {
#if defined(TLRENDER_GL)
            tests.push_back(gl_tests::MeshTest::create(context));
#endif // TLRENDER_GL
        }
        if (1)
        {
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
            tests.push_back(qt_tests::TimeObjectTest::create(context));
#endif // TLRENDER_QT5 || TLRENDER_QT6
        }
    }
    for (const auto& i : tests)
    {
        std::cout << "Running test: " << i->getName() << std::endl;
        i->run();

        context->tick();
    }

    return 0;
}
