// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
#include <tlQtTest/TimeObjectTest.h>
#include <tlQt/Init.h>
#endif // TLRENDER_QT5 || TLRENDER_QT6

#include <tlTimelineTest/ColorOptionsTest.h>
#include <tlTimelineTest/CompareOptionsTest.h>
#include <tlTimelineTest/DisplayOptionsTest.h>
#include <tlTimelineTest/MemoryReferenceTest.h>
#include <tlTimelineTest/PlayerOptionsTest.h>
#include <tlTimelineTest/PlayerTest.h>
#include <tlTimelineTest/TimelineTest.h>
#include <tlTimelineTest/UtilTest.h>

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
#if defined(TLRENDER_STB)
#include <tlIOTest/STBTest.h>
#endif // TLRENDER_STB

#include <tlCoreTest/AudioTest.h>
#include <tlCoreTest/FileInfoTest.h>
#include <tlCoreTest/HDRTest.h>
#include <tlCoreTest/PathTest.h>
#include <tlCoreTest/TimeTest.h>
#include <tlCoreTest/URLTest.h>

#include <tlTimeline/Init.h>

#include <ftk/Core/Context.h>

#include <iostream>
#include <vector>

using namespace tl;
using namespace tl::tests;

void coreTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(core_tests::AudioTest::create(context));
    tests.push_back(core_tests::FileInfoTest::create(context));
    tests.push_back(core_tests::HDRTest::create(context));
    tests.push_back(core_tests::PathTest::create(context));
    tests.push_back(core_tests::TimeTest::create(context));
    tests.push_back(core_tests::URLTest::create(context));
}

void ioTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
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
#if defined(TLRENDER_STB)
    tests.push_back(io_tests::STBTest::create(context));
#endif // TLRENDER_STB
}

void timelineTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(timeline_tests::ColorOptionsTest::create(context));
    tests.push_back(timeline_tests::CompareOptionsTest::create(context));
    tests.push_back(timeline_tests::DisplayOptionsTest::create(context));
    tests.push_back(timeline_tests::MemoryReferenceTest::create(context));
    tests.push_back(timeline_tests::PlayerOptionsTest::create(context));
    tests.push_back(timeline_tests::PlayerTest::create(context));
    tests.push_back(timeline_tests::TimelineTest::create(context));
    tests.push_back(timeline_tests::UtilTest::create(context));
}

void qtTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
    tests.push_back(qt_tests::TimeObjectTest::create(context));
#endif // TLRENDER_QT5 || TLRENDER_QT6
}

int main(int argc, char* argv[])
{
    auto context = ftk::Context::create();
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
    qt::init(
        context,
        qt::DefaultSurfaceFormat::OpenGL_4_1_CoreProfile);
#else // TLRENDER_QT5 || TLRENDER_QT6
    timeline::init(context);
#endif // TLRENDER_QT5 || TLRENDER_QT6

    auto logObserver = ftk::ListObserver<ftk::LogItem>::create(
        context->getSystem<ftk::LogSystem>()->observeLogItems(),
        [](const std::vector<ftk::LogItem>& value)
        {
            for (const auto& i : value)
            {
                std::cout << "[LOG] " << toString(i) << std::endl;
            }
        },
        ftk::ObserverAction::Suppress);

    context->tick();

    std::vector<std::shared_ptr<tests::ITest> > tests;
    //tests.push_back(core_tests::URLTest::create(context));
    coreTests(tests, context);
    ioTests(tests, context);
    timelineTests(tests, context);
    qtTests(tests, context);

    for (const auto& test : tests)
    {
        std::cout << "Running test: " << test->getName() << std::endl;
        test->run();
        context->tick();
    }

    std::cout << "Finished tests" << std::endl;
    return 0;
}
