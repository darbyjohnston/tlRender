// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/EditTest.h>

#include <tlTimeline/Edit.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

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
            otio::SerializableObject::Retainer<otio::Composable> getChild(
                const otio::SerializableObject::Retainer<otio::Timeline>& otioTimeline,
                int track,
                int index)
            {
                auto otioTrack = otio::dynamic_retainer_cast<otio::Track>(otioTimeline->tracks()->children()[track]);
                return otioTrack->children()[index];
            }

            otio::SerializableObject::Retainer<otio::Clip> getClip(
                const otio::SerializableObject::Retainer<otio::Timeline>& otioTimeline,
                int track,
                int index)
            {
                auto otioTrack = otio::dynamic_retainer_cast<otio::Track>(otioTimeline->tracks()->children()[track]);
                return otio::dynamic_retainer_cast<otio::Clip>(otioTrack->children()[index]);
            }
        }

        void EditTest::run()
        {
            _insert();
        }

        void EditTest::_insert()
        {
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                auto otioTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));

                InsertData insertData;
                insertData.composable = getChild(otioTimeline, 0, 0);
                insertData.trackIndex = 0;
                insertData.insertIndex = 0;
                auto otioTimeline2 = insert(otioTimeline, { insertData });
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline2, 0, 0)->name());

                insertData.composable = getChild(otioTimeline2, 0, 0);
                insertData.insertIndex = 1;
                auto otioTimeline3 = insert(otioTimeline2, { insertData });
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline3, 0, 0)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                auto otioTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 1",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));

                InsertData insertData;
                insertData.composable = getChild(otioTimeline, 0, 0);
                insertData.trackIndex = 0;
                insertData.insertIndex = 2;
                auto otioTimeline2 = insert(otioTimeline, { insertData });
                TLRENDER_ASSERT("Video 1" == getChild(otioTimeline2, 0, 0)->name());
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline2, 0, 1)->name());

                insertData.composable = getChild(otioTimeline2, 0, 1);
                insertData.insertIndex = 0;
                auto otioTimeline3 = insert(otioTimeline2, { insertData });
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline3, 0, 0)->name());
                TLRENDER_ASSERT("Video 1" == getChild(otioTimeline3, 0, 1)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                auto otioTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 1",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 2",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));

                InsertData insertData;
                insertData.composable = getChild(otioTimeline, 0, 2);
                insertData.trackIndex = 0;
                insertData.insertIndex = 0;
                auto otioTimeline2 = insert(otioTimeline, { insertData });
                TLRENDER_ASSERT("Video 2" == getChild(otioTimeline2, 0, 0)->name());
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline2, 0, 1)->name());
                TLRENDER_ASSERT("Video 1" == getChild(otioTimeline2, 0, 2)->name());

                insertData.composable = getChild(otioTimeline2, 0, 1);
                insertData.insertIndex = 3;
                auto otioTimeline3 = insert(otioTimeline2, { insertData });
                TLRENDER_ASSERT("Video 2" == getChild(otioTimeline3, 0, 0)->name());
                TLRENDER_ASSERT("Video 1" == getChild(otioTimeline3, 0, 1)->name());
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline3, 0, 2)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                auto otioTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 1",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 2",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack = new otio::Track("Audio", otio::nullopt, otio::Track::Kind::audio);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Audio 0",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 48000.0),
                        otime::RationalTime(48000.0, 48000.0))));
                otioTrack->append_child(new otio::Clip(
                    "Audio 1",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 48000.0),
                        otime::RationalTime(48000.0, 48000.0))));
                otioTrack->append_child(new otio::Clip(
                    "Audio 2",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 48000.0),
                        otime::RationalTime(48000.0, 48000.0))));

                std::vector<InsertData> insertData;
                insertData.push_back({ getChild(otioTimeline, 0, 2), 0, 0 });
                insertData.push_back({ getChild(otioTimeline, 1, 2), 1, 0 });
                auto otioTimeline2 = insert(otioTimeline, insertData);
                TLRENDER_ASSERT("Video 2" == getChild(otioTimeline2, 0, 0)->name());
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline2, 0, 1)->name());
                TLRENDER_ASSERT("Video 1" == getChild(otioTimeline2, 0, 2)->name());
                TLRENDER_ASSERT("Audio 2" == getChild(otioTimeline2, 1, 0)->name());
                TLRENDER_ASSERT("Audio 0" == getChild(otioTimeline2, 1, 1)->name());
                TLRENDER_ASSERT("Audio 1" == getChild(otioTimeline2, 1, 2)->name());

                insertData.clear();
                insertData.push_back({ getChild(otioTimeline, 0, 1), 0, 3 });
                insertData.push_back({ getChild(otioTimeline, 1, 1), 1, 3 });
                auto otioTimeline3 = insert(otioTimeline2, insertData);
                TLRENDER_ASSERT("Video 2" == getChild(otioTimeline3, 0, 0)->name());
                TLRENDER_ASSERT("Video 1" == getChild(otioTimeline3, 0, 1)->name());
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline3, 0, 2)->name());
                TLRENDER_ASSERT("Audio 2" == getChild(otioTimeline3, 1, 0)->name());
                TLRENDER_ASSERT("Audio 1" == getChild(otioTimeline3, 1, 1)->name());
                TLRENDER_ASSERT("Audio 0" == getChild(otioTimeline3, 1, 2)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                auto otioTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0",
                    nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack = new otio::Track("Video", otio::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);

                InsertData insertData;
                insertData.composable = getChild(otioTimeline, 0, 0);
                insertData.trackIndex = 1;
                insertData.insertIndex = 0;
                auto otioTimeline2 = insert(otioTimeline, { insertData });
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline2, 1, 0)->name());

                insertData.composable = getChild(otioTimeline2, 1, 0);
                insertData.trackIndex = 0;
                auto otioTimeline3 = insert(otioTimeline2, { insertData });
                TLRENDER_ASSERT("Video 0" == getChild(otioTimeline3, 0, 0)->name());
            }
        }
    }
}
