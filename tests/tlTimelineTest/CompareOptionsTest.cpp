// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/CompareOptionsTest.h>

#include <tlTimeline/CompareOptions.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        CompareOptionsTest::CompareOptionsTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::CompareOptionsTest")
        {}

        std::shared_ptr<CompareOptionsTest> CompareOptionsTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<CompareOptionsTest>(new CompareOptionsTest(context));
        }

        void CompareOptionsTest::run()
        {
            {
                _enum<Compare>("Compare", getCompareEnums);
                _enum<CompareTime>("CompareTime", getCompareTimeEnums);
            }
            {
                CompareOptions options;
                options.compare = Compare::B;
                FTK_ASSERT(options == options);
                FTK_ASSERT(options != CompareOptions());
            }
            {
                const std::vector<ftk::ImageInfo> infos =
                {
                    ftk::ImageInfo(1920, 1080, ftk::ImageType::RGBA_U8),
                    ftk::ImageInfo(1920 / 2, 1080 / 2, ftk::ImageType::RGBA_U8),
                    ftk::ImageInfo(1920 / 2, 1080 / 2, ftk::ImageType::RGBA_U8),
                    ftk::ImageInfo(1920 / 2, 1080 / 2, ftk::ImageType::RGBA_U8)
                };

                for (auto compare :
                    {
                        Compare::A,
                        Compare::B,
                        Compare::Wipe,
                        Compare::Overlay,
                        Compare::Difference
                    })
                {
                    auto boxes = getBoxes(compare, infos);
                    FTK_ASSERT(2 == boxes.size());
                    FTK_ASSERT(ftk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                    FTK_ASSERT(ftk::Box2I(0, 0, 1920, 1080) == boxes[1]);
                    auto renderSize = getRenderSize(compare, infos);
                    FTK_ASSERT(ftk::Size2I(1920, 1080) == renderSize);
                }

                auto boxes = getBoxes(Compare::Horizontal, infos);
                FTK_ASSERT(2 == boxes.size());
                FTK_ASSERT(ftk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                FTK_ASSERT(ftk::Box2I(1920, 0, 1920, 1080) == boxes[1]);
                auto renderSize = getRenderSize(Compare::Horizontal, infos);
                FTK_ASSERT(ftk::Size2I(1920 * 2, 1080) == renderSize);

                boxes = getBoxes(Compare::Vertical, infos);
                FTK_ASSERT(2 == boxes.size());
                FTK_ASSERT(ftk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                FTK_ASSERT(ftk::Box2I(0, 1080, 1920, 1080) == boxes[1]);
                renderSize = getRenderSize(Compare::Vertical, infos);
                FTK_ASSERT(ftk::Size2I(1920, 1080 * 2) == renderSize);

                boxes = getBoxes(Compare::Tile, infos);
                FTK_ASSERT(4 == boxes.size());
                FTK_ASSERT(ftk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                FTK_ASSERT(ftk::Box2I(1920, 0, 1920, 1080) == boxes[1]);
                FTK_ASSERT(ftk::Box2I(0, 1080, 1920, 1080) == boxes[2]);
                FTK_ASSERT(ftk::Box2I(1920, 1080, 1920, 1080) == boxes[3]);
                renderSize = getRenderSize(Compare::Tile, infos);
                FTK_ASSERT(ftk::Size2I(1920 * 2, 1080 * 2) == renderSize);
            }
            {
                const auto time = getCompareTime(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    CompareTime::Absolute);
                FTK_ASSERT(time == OTIO_NS::RationalTime(0.0, 24.0));
            }
            {
                const auto time = getCompareTime(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(24.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    CompareTime::Relative);
                FTK_ASSERT(time == OTIO_NS::RationalTime(24.0, 24.0));
            }
        }
    }
}
