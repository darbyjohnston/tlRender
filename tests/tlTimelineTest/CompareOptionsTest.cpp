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
        CompareOptionsTest::CompareOptionsTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "timeline_tests::CompareOptionsTest")
        {}

        std::shared_ptr<CompareOptionsTest> CompareOptionsTest::create(const std::shared_ptr<feather_tk::Context>& context)
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
                FEATHER_TK_ASSERT(options == options);
                FEATHER_TK_ASSERT(options != CompareOptions());
            }
            {
                const std::vector<feather_tk::ImageInfo> infos =
                {
                    feather_tk::ImageInfo(1920, 1080, feather_tk::ImageType::RGBA_U8),
                    feather_tk::ImageInfo(1920 / 2, 1080 / 2, feather_tk::ImageType::RGBA_U8),
                    feather_tk::ImageInfo(1920 / 2, 1080 / 2, feather_tk::ImageType::RGBA_U8),
                    feather_tk::ImageInfo(1920 / 2, 1080 / 2, feather_tk::ImageType::RGBA_U8)
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
                    FEATHER_TK_ASSERT(2 == boxes.size());
                    FEATHER_TK_ASSERT(feather_tk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                    FEATHER_TK_ASSERT(feather_tk::Box2I(0, 0, 1920, 1080) == boxes[1]);
                    auto renderSize = getRenderSize(compare, infos);
                    FEATHER_TK_ASSERT(feather_tk::Size2I(1920, 1080) == renderSize);
                }

                auto boxes = getBoxes(Compare::Horizontal, infos);
                FEATHER_TK_ASSERT(2 == boxes.size());
                FEATHER_TK_ASSERT(feather_tk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                FEATHER_TK_ASSERT(feather_tk::Box2I(1920, 0, 1920, 1080) == boxes[1]);
                auto renderSize = getRenderSize(Compare::Horizontal, infos);
                FEATHER_TK_ASSERT(feather_tk::Size2I(1920 * 2, 1080) == renderSize);

                boxes = getBoxes(Compare::Vertical, infos);
                FEATHER_TK_ASSERT(2 == boxes.size());
                FEATHER_TK_ASSERT(feather_tk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                FEATHER_TK_ASSERT(feather_tk::Box2I(0, 1080, 1920, 1080) == boxes[1]);
                renderSize = getRenderSize(Compare::Vertical, infos);
                FEATHER_TK_ASSERT(feather_tk::Size2I(1920, 1080 * 2) == renderSize);

                boxes = getBoxes(Compare::Tile, infos);
                FEATHER_TK_ASSERT(4 == boxes.size());
                FEATHER_TK_ASSERT(feather_tk::Box2I(0, 0, 1920, 1080) == boxes[0]);
                FEATHER_TK_ASSERT(feather_tk::Box2I(1920, 0, 1920, 1080) == boxes[1]);
                FEATHER_TK_ASSERT(feather_tk::Box2I(0, 1080, 1920, 1080) == boxes[2]);
                FEATHER_TK_ASSERT(feather_tk::Box2I(1920, 1080, 1920, 1080) == boxes[3]);
                renderSize = getRenderSize(Compare::Tile, infos);
                FEATHER_TK_ASSERT(feather_tk::Size2I(1920 * 2, 1080 * 2) == renderSize);
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
                FEATHER_TK_ASSERT(time == OTIO_NS::RationalTime(0.0, 24.0));
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
                FEATHER_TK_ASSERT(time == OTIO_NS::RationalTime(24.0, 24.0));
            }
        }
    }
}
