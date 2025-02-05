// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/CompareOptionsTest.h>

#include <tlTimeline/CompareOptions.h>

#include <dtk/core/Assert.h>
#include <dtk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        CompareOptionsTest::CompareOptionsTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "timeline_tests::CompareOptionsTest")
        {}

        std::shared_ptr<CompareOptionsTest> CompareOptionsTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<CompareOptionsTest>(new CompareOptionsTest(context));
        }

        void CompareOptionsTest::run()
        {
            {
                _enum<CompareMode>("CompareMode", getCompareModeEnums);
                _enum<CompareTimeMode>("CompareTimeMode", getCompareTimeModeEnums);
            }
            {
                CompareOptions options;
                options.mode = CompareMode::B;
                DTK_ASSERT(options == options);
                DTK_ASSERT(options != CompareOptions());
            }
            {
                const std::vector<dtk::ImageInfo> infos =
                {
                    dtk::ImageInfo(1920, 1080, dtk::ImageType::RGBA_U8),
                    dtk::ImageInfo(1920 / 2, 1080 / 2, dtk::ImageType::RGBA_U8),
                    dtk::ImageInfo(1920 / 2, 1080 / 2, dtk::ImageType::RGBA_U8),
                    dtk::ImageInfo(1920 / 2, 1080 / 2, dtk::ImageType::RGBA_U8)
                };

                for (auto mode :
                    {
                        CompareMode::A,
                        CompareMode::B,
                        CompareMode::Wipe,
                        CompareMode::Overlay,
                        CompareMode::Difference
                    })
                {
                    auto boxes = getBoxes(mode, infos);
                    DTK_ASSERT(2 == boxes.size());
                    DTK_ASSERT(dtk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                    DTK_ASSERT(dtk::Box2I(0, 0, 1920, 1080) == boxes[1]);
                    auto renderSize = getRenderSize(mode, infos);
                    DTK_ASSERT(dtk::Size2I(1920, 1080) == renderSize);
                }

                auto boxes = getBoxes(CompareMode::Horizontal, infos);
                DTK_ASSERT(2 == boxes.size());
                DTK_ASSERT(dtk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                DTK_ASSERT(dtk::Box2I(1920, 0, 1920, 1080) == boxes[1]);
                auto renderSize = getRenderSize(CompareMode::Horizontal, infos);
                DTK_ASSERT(dtk::Size2I(1920 * 2, 1080) == renderSize);

                boxes = getBoxes(CompareMode::Vertical, infos);
                DTK_ASSERT(2 == boxes.size());
                DTK_ASSERT(dtk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                DTK_ASSERT(dtk::Box2I(0, 1080, 1920, 1080) == boxes[1]);
                renderSize = getRenderSize(CompareMode::Vertical, infos);
                DTK_ASSERT(dtk::Size2I(1920, 1080 * 2) == renderSize);

                boxes = getBoxes(CompareMode::Tile, infos);
                DTK_ASSERT(4 == boxes.size());
                DTK_ASSERT(dtk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                DTK_ASSERT(dtk::Box2I(1920, 0, 1920, 1080) == boxes[1]);
                DTK_ASSERT(dtk::Box2I(0, 1080, 1920, 1080) == boxes[2]);
                DTK_ASSERT(dtk::Box2I(1920, 1080, 1920, 1080) == boxes[3]);
                renderSize = getRenderSize(CompareMode::Tile, infos);
                DTK_ASSERT(dtk::Size2I(1920 * 2, 1080 * 2) == renderSize);
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
                    CompareTimeMode::Absolute);
                DTK_ASSERT(time == OTIO_NS::RationalTime(0.0, 24.0));
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
                    CompareTimeMode::Relative);
                DTK_ASSERT(time == OTIO_NS::RationalTime(24.0, 24.0));
            }
        }
    }
}
