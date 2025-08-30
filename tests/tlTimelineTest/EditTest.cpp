// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/EditTest.h>

#include <tlTimeline/Edit.h>
#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>

#include <tlCore/Path.h>

#include <opentimelineio/clip.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        EditTest::EditTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::EditTest")
        {}

        std::shared_ptr<EditTest> EditTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<EditTest>(new EditTest(context));
        }

        namespace
        {
            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Composable> getChild(
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& otioTimeline,
                int track,
                int index)
            {
                auto otioTrack = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(otioTimeline->tracks()->children()[track]);
                return otioTrack->children()[index];
            }

            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip> getClip(
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& otioTimeline,
                int track,
                int index)
            {
                auto otioTrack = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(otioTimeline->tracks()->children()[track]);
                return OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(otioTrack->children()[index]);
            }
        }

        void EditTest::run()
        {
            _move();
        }

        void EditTest::_move()
        {
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline(new OTIO_NS::Timeline);
                auto otioTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 0",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                auto otioTimeline2 = move(otioTimeline, { moveData });
                FTK_ASSERT("Video 0" == getChild(otioTimeline2, 0, 0)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 1;
                auto otioTimeline3 = move(otioTimeline2, { moveData });
                FTK_ASSERT("Video 0" == getChild(otioTimeline3, 0, 0)->name());
            }
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline(new OTIO_NS::Timeline);
                auto otioTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 0",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 1",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 2;
                auto otioTimeline2 = move(otioTimeline, { moveData });
                FTK_ASSERT("Video 1" == getChild(otioTimeline2, 0, 0)->name());
                FTK_ASSERT("Video 0" == getChild(otioTimeline2, 0, 1)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 1;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                auto otioTimeline3 = move(otioTimeline2, { moveData });
                FTK_ASSERT("Video 0" == getChild(otioTimeline3, 0, 0)->name());
                FTK_ASSERT("Video 1" == getChild(otioTimeline3, 0, 1)->name());
            }
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline(new OTIO_NS::Timeline);
                auto otioTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 0",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 1",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 2",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 2;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                auto otioTimeline2 = move(otioTimeline, { moveData });
                FTK_ASSERT("Video 2" == getChild(otioTimeline2, 0, 0)->name());
                FTK_ASSERT("Video 0" == getChild(otioTimeline2, 0, 1)->name());
                FTK_ASSERT("Video 1" == getChild(otioTimeline2, 0, 2)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 1;
                moveData.toTrack = 0;
                moveData.toIndex = 3;
                auto otioTimeline3 = move(otioTimeline2, { moveData });
                FTK_ASSERT("Video 2" == getChild(otioTimeline3, 0, 0)->name());
                FTK_ASSERT("Video 1" == getChild(otioTimeline3, 0, 1)->name());
                FTK_ASSERT("Video 0" == getChild(otioTimeline3, 0, 2)->name());
            }
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline(new OTIO_NS::Timeline);
                auto otioTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 0",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 1",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 2",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));
                otioTrack = new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Audio 0",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 48000.0),
                        OTIO_NS::RationalTime(48000.0, 48000.0))));
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Audio 1",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 48000.0),
                        OTIO_NS::RationalTime(48000.0, 48000.0))));
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Audio 2",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 48000.0),
                        OTIO_NS::RationalTime(48000.0, 48000.0))));

                std::vector<MoveData> moveData;
                moveData.push_back({ 0, 2, 0, 0 });
                moveData.push_back({ 1, 2, 1, 0 });
                auto otioTimeline2 = move(otioTimeline, moveData);
                FTK_ASSERT("Video 2" == getChild(otioTimeline2, 0, 0)->name());
                FTK_ASSERT("Video 0" == getChild(otioTimeline2, 0, 1)->name());
                FTK_ASSERT("Video 1" == getChild(otioTimeline2, 0, 2)->name());
                FTK_ASSERT("Audio 2" == getChild(otioTimeline2, 1, 0)->name());
                FTK_ASSERT("Audio 0" == getChild(otioTimeline2, 1, 1)->name());
                FTK_ASSERT("Audio 1" == getChild(otioTimeline2, 1, 2)->name());

                moveData.clear();
                moveData.push_back({ 0, 1, 0, 3 });
                moveData.push_back({ 1, 1, 1, 3 });
                auto otioTimeline3 = move(otioTimeline2, moveData);
                FTK_ASSERT("Video 2" == getChild(otioTimeline3, 0, 0)->name());
                FTK_ASSERT("Video 1" == getChild(otioTimeline3, 0, 1)->name());
                FTK_ASSERT("Video 0" == getChild(otioTimeline3, 0, 2)->name());
                FTK_ASSERT("Audio 2" == getChild(otioTimeline3, 1, 0)->name());
                FTK_ASSERT("Audio 1" == getChild(otioTimeline3, 1, 1)->name());
                FTK_ASSERT("Audio 0" == getChild(otioTimeline3, 1, 2)->name());
            }
            {
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline(new OTIO_NS::Timeline);
                auto otioTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new OTIO_NS::Clip(
                    "Video 0",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0))));
                otioTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.toTrack = 1;
                moveData.toIndex = 0;
                auto otioTimeline2 = move(otioTimeline, { moveData });
                FTK_ASSERT("Video 0" == getChild(otioTimeline2, 1, 0)->name());

                moveData.fromTrack = 1;
                moveData.fromIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                auto otioTimeline3 = move(otioTimeline2, { moveData });
                FTK_ASSERT("Video 0" == getChild(otioTimeline3, 0, 0)->name());
            }
            for (const auto otio : { "SingleClip.otio", "SingleClipSeq.otio" })
            {
                for (const auto toMemoryReference : { ToMemoryReference::Shared, ToMemoryReference::Raw })
                {
                    auto otioTimeline = timeline::create(
                        _context,
                        file::Path(TLRENDER_SAMPLE_DATA, otio));
                    auto track = dynamic_cast<OTIO_NS::Track*>(otioTimeline->tracks()->children()[0].value);
                    track->append_child(new OTIO_NS::Clip(
                        "Video",
                        nullptr,
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, 30.0),
                            OTIO_NS::RationalTime(30.0, 30.0))));
                    toMemoryReferences(otioTimeline.value, TLRENDER_SAMPLE_DATA, toMemoryReference);

                    const std::string video0 = getChild(otioTimeline, 0, 0)->name();
                    const std::string video1 = getChild(otioTimeline, 0, 1)->name();

                    MoveData moveData;
                    moveData.fromTrack = 0;
                    moveData.fromIndex = 0;
                    moveData.toTrack = 0;
                    moveData.toIndex = 2;
                    auto otioTimeline2 = move(otioTimeline, { moveData });
                    FTK_ASSERT(video1 == getChild(otioTimeline2, 0, 0)->name());
                    FTK_ASSERT(video0 == getChild(otioTimeline2, 0, 1)->name());

                    moveData.fromTrack = 0;
                    moveData.fromIndex = 1;
                    moveData.toTrack = 0;
                    moveData.toIndex = 0;
                    auto otioTimeline3 = move(otioTimeline2, { moveData });
                    FTK_ASSERT(video0 == getChild(otioTimeline3, 0, 0)->name());
                    FTK_ASSERT(video1 == getChild(otioTimeline3, 0, 1)->name());
                    
                    if (ToMemoryReference::Raw == toMemoryReference)
                    {
                        for (const auto clip : otioTimeline->find_clips())
                        {
                            if (const auto ref = dynamic_cast<RawMemoryReference*>(clip->media_reference()))
                            {
                                delete [] ref->memory();
                                ref->set_memory(nullptr, 0);
                            }
                            else if (const auto ref = dynamic_cast<RawMemorySequenceReference*>(clip->media_reference()))
                            {
                                for (const auto& memory : ref->memory())
                                {
                                    delete [] memory;
                                }
                                ref->set_memory({}, {});
                            }
                        }
                    }
                }
            }
            for (const auto otioz : { "SingleClip.otioz", "SingleClipSeq.otioz" })
            {
                auto otioTimeline = timeline::create(
                    _context,
                    file::Path(TLRENDER_SAMPLE_DATA, otioz));
                auto track = dynamic_cast<OTIO_NS::Track*>(otioTimeline->tracks()->children()[0].value);
                track->append_child(new OTIO_NS::Clip(
                    "Video",
                    nullptr,
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 30.0),
                        OTIO_NS::RationalTime(30.0, 30.0))));

                const std::string video0 = getChild(otioTimeline, 0, 0)->name();
                const std::string video1 = getChild(otioTimeline, 0, 1)->name();

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 2;
                auto otioTimeline2 = move(otioTimeline, { moveData });
                FTK_ASSERT(video1 == getChild(otioTimeline2, 0, 0)->name());
                FTK_ASSERT(video0 == getChild(otioTimeline2, 0, 1)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 1;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                auto otioTimeline3 = move(otioTimeline2, { moveData });
                FTK_ASSERT(video0 == getChild(otioTimeline3, 0, 0)->name());
                FTK_ASSERT(video1 == getChild(otioTimeline3, 0, 1)->name());
            }
        }
    }
}
