// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ImageTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Image.h>

namespace tlr
{
    namespace CoreTest
    {
        ImageTest::ImageTest() :
            ITest("CoreTest::ImageTest")
        {}

        std::shared_ptr<ImageTest> ImageTest::create()
        {
            return std::shared_ptr<ImageTest>(new ImageTest);
        }

        void ImageTest::run()
        {
        }
    }
}
