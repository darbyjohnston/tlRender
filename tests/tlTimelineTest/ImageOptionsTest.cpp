// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/ImageOptionsTest.h>

#include <tlTimeline/ImageOptions.h>

#include <dtk/core/Assert.h>
#include <dtk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        ImageOptionsTest::ImageOptionsTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "timeline_tests::ImageOptionsTest")
        {}

        std::shared_ptr<ImageOptionsTest> ImageOptionsTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<ImageOptionsTest>(new ImageOptionsTest(context));
        }

        void ImageOptionsTest::run()
        {
            {
                _enum<InputVideoLevels>("InputVideoLevels", getInputVideoLevelsEnums);
                _enum<AlphaBlend>("AlphaBlend", getAlphaBlendEnums);
                _enum<ImageFilter>("ImageFilter", getImageFilterEnums);
            }
            {
                ImageFilters v;
                v.minify = ImageFilter::Nearest;
                DTK_ASSERT(v == v);
                DTK_ASSERT(v != ImageFilters());
            }
            {
                ImageOptions v;
                v.videoLevels = InputVideoLevels::FullRange;
                DTK_ASSERT(v == v);
                DTK_ASSERT(v != ImageOptions());
            }
        }
    }
}
