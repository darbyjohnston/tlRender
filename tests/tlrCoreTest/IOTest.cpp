// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/IOTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/IO.h>
#include <tlrCore/StringFormat.h>

#include <sstream>

using namespace tlr::imaging;
using namespace tlr::io;

namespace tlr
{
    namespace CoreTest
    {
        IOTest::IOTest() :
            ITest("CoreTest::IOTest")
        {}

        std::shared_ptr<IOTest> IOTest::create()
        {
            return std::shared_ptr<IOTest>(new IOTest);
        }

        void IOTest::run()
        {
            _videoFrame();
            _ioSystem();
        }

        void IOTest::_videoFrame()
        {
            {
                const VideoFrame f;
                TLR_ASSERT(invalidTime == f.time);
                TLR_ASSERT(!f.image);
            }
            {
                const auto time = otime::RationalTime(1.0, 24.0);
                const auto image = Image::create(imaging::Info(160, 80, PixelType::L_U8));
                const VideoFrame f(time, image);
                TLR_ASSERT(time == f.time);
                TLR_ASSERT(image == f.image);
            }
            {
                const auto time = otime::RationalTime(1.0, 24.0);
                const auto image = Image::create(imaging::Info(16, 16, PixelType::L_U8));
                const VideoFrame a(time, image);
                VideoFrame b(time, image);
                TLR_ASSERT(a == b);
                b.time = otime::RationalTime(2.0, 24.0);
                TLR_ASSERT(a != b);
                TLR_ASSERT(a < b);
            }
        }

        void IOTest::_ioSystem()
        {
            auto system = System::create();
            for (const auto& plugin : system->getPlugins())
            {
                std::stringstream ss;
                ss << "Plugin: " << plugin->getName();
                _print(ss.str());
            }
        }
    }
}
