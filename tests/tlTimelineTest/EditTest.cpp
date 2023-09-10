// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/EditTest.h>

#include <tlTimeline/Edit.h>

#include <tlCore/Assert.h>

#include <opentimelineio/clip.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        EditTest::EditTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::EditTest", context)
        {}

        std::shared_ptr<EditTest> EditTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<EditTest>(new EditTest(context));
        }

        namespace
        {
            otio::SerializableObject::Retainer<otio::Timeline> createTimeline()
            {
                auto otioTimeline = new otio::Timeline;
                auto otioVideoTrack = new otio::Track(
                    "Video",
                    otio::nullopt,
                    otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioVideoTrack);
                auto otioClip0 = new otio::Clip("Clip 0");
                otioClip0->set_source_range(otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(24.0, 24.0)));
                otioVideoTrack->append_child(otioClip0);
                auto otioClip1 = new otio::Clip("Clip 1");
                otioClip1->set_source_range(otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(24.0, 24.0)));
                otioVideoTrack->append_child(otioClip1);
                auto otioClip2 = new otio::Clip("Clip 2");
                otioClip2->set_source_range(otime::TimeRange(
                    otime::RationalTime(0.0, 24.0),
                    otime::RationalTime(24.0, 24.0)));
                otioVideoTrack->append_child(otioClip2);
                return otioTimeline;
            }
        }

        void EditTest::run()
        {
            {
                auto otioTimeline = createTimeline();

                InsertData insertData;
                auto otioVideoTrack = dynamic_cast<otio::Track*>(otioTimeline->tracks()->children()[0].value);
                insertData.composable = otioVideoTrack->children()[2];
                insertData.trackIndex = 0;
                insertData.insertIndex = 0;
                auto otioTimeline2 = insert(otioTimeline, { insertData });

                auto otioVideoTrack2 = dynamic_cast<otio::Track*>(otioTimeline2->tracks()->children()[0].value);
                const auto& children = otioVideoTrack2->children();
                TLRENDER_ASSERT("Clip 2" == children[0]->name());
                TLRENDER_ASSERT("Clip 0" == children[1]->name());
                TLRENDER_ASSERT("Clip 1" == children[2]->name());
            }
            {
                auto otioTimeline = createTimeline();

                InsertData insertData;
                auto otioVideoTrack = dynamic_cast<otio::Track*>(otioTimeline->tracks()->children()[0].value);
                insertData.composable = otioVideoTrack->children()[0];
                insertData.trackIndex = 0;
                insertData.insertIndex = 3;
                auto otioTimeline2 = insert(otioTimeline, { insertData });

                auto otioVideoTrack2 = dynamic_cast<otio::Track*>(otioTimeline2->tracks()->children()[0].value);
                const auto& children = otioVideoTrack2->children();
                TLRENDER_ASSERT("Clip 1" == children[0]->name());
                TLRENDER_ASSERT("Clip 2" == children[1]->name());
                TLRENDER_ASSERT("Clip 0" == children[2]->name());
            }
        }
    }
}
