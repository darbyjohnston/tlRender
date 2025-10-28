// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/UtilTest.h>

#include <tlTimeline/Util.h>

#include <tlCore/FileInfo.h>

#include <ftk/Core/Format.h>

#include <opentimelineio/clip.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        UtilTest::UtilTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::UtilTest")
        {}

        std::shared_ptr<UtilTest> UtilTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<UtilTest>(new UtilTest(context));
        }

        void UtilTest::run()
        {
            _enums();
            _extensions();
            _ranges();
            _loop();
            _util();
            _audio();
            _otioz();
        }

        void UtilTest::_enums()
        {
            _enum<CacheDirection>("CacheDirection", getCacheDirectionEnums);
            _enum<ToMemoryReference>("ToMemoryReference", getToMemoryReferenceEnums);
        }

        void UtilTest::_extensions()
        {
            for (const auto& i : getExtensions(
                _context,
                static_cast<int>(io::FileType::Media) |
                static_cast<int>(io::FileType::Sequence)))
            {
                std::stringstream ss;
                ss << "Timeline extension: " << i;
                _print(ss.str());
            }
            for (const auto& path : getPaths(
                _context,
                file::Path(TLRENDER_SAMPLE_DATA),
                file::PathOptions()))
            {
                _print(ftk::Format("Path: {0}").arg(path.get()));
            }
        }

        void UtilTest::_ranges()
        {
            {
                std::vector<OTIO_NS::RationalTime> f;
                auto r = toRanges(f);
                FTK_ASSERT(r.empty());
            }
            {
                std::vector<OTIO_NS::RationalTime> f =
                {
                    OTIO_NS::RationalTime(0, 24)
                };
                auto r = toRanges(f);
                FTK_ASSERT(1 == r.size());
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(0, 24), OTIO_NS::RationalTime(1, 24)) == r[0]);
            }
            {
                std::vector<OTIO_NS::RationalTime> f =
                {
                    OTIO_NS::RationalTime(0, 24),
                    OTIO_NS::RationalTime(1, 24)
                };
                auto r = toRanges(f);
                FTK_ASSERT(1 == r.size());
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(0, 24), OTIO_NS::RationalTime(2, 24)) == r[0]);
            }
            {
                std::vector<OTIO_NS::RationalTime> f =
                {
                    OTIO_NS::RationalTime(0, 24),
                    OTIO_NS::RationalTime(1, 24),
                    OTIO_NS::RationalTime(2, 24)
                };
                auto r = toRanges(f);
                FTK_ASSERT(1 == r.size());
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(0, 24), OTIO_NS::RationalTime(3, 24)) == r[0]);
            }
            {
                std::vector<OTIO_NS::RationalTime> f =
                {
                    OTIO_NS::RationalTime(0, 24),
                    OTIO_NS::RationalTime(2, 24)
                };
                auto r = toRanges(f);
                FTK_ASSERT(2 == r.size());
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(0, 24), OTIO_NS::RationalTime(1, 24)) == r[0]);
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(2, 24), OTIO_NS::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<OTIO_NS::RationalTime> f =
                {
                    OTIO_NS::RationalTime(0, 24),
                    OTIO_NS::RationalTime(1, 24),
                    OTIO_NS::RationalTime(3, 24)
                };
                auto r = toRanges(f);
                FTK_ASSERT(2 == r.size());
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(0, 24), OTIO_NS::RationalTime(2, 24)) == r[0]);
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(3, 24), OTIO_NS::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<OTIO_NS::RationalTime> f =
                {
                    OTIO_NS::RationalTime(0, 24),
                    OTIO_NS::RationalTime(1, 24),
                    OTIO_NS::RationalTime(3, 24),
                    OTIO_NS::RationalTime(4, 24)
                };
                auto r = toRanges(f);
                FTK_ASSERT(2 == r.size());
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(0, 24), OTIO_NS::RationalTime(2, 24)) == r[0]);
                FTK_ASSERT(OTIO_NS::TimeRange(OTIO_NS::RationalTime(3, 24), OTIO_NS::RationalTime(2, 24)) == r[1]);
            }
        }

        void UtilTest::_loop()
        {
            {
                const OTIO_NS::TimeRange timeRange(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::RationalTime(24.0, 24.0));
                bool looped = false;
                OTIO_NS::RationalTime t = loop(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    timeRange,
                    &looped);
                FTK_ASSERT(OTIO_NS::RationalTime(0.0, 24.0) == t);
                FTK_ASSERT(!looped);
                t = loop(
                    OTIO_NS::RationalTime(24.0, 24.0),
                    timeRange,
                    &looped);
                FTK_ASSERT(OTIO_NS::RationalTime(0.0, 24.0) == t);
                FTK_ASSERT(looped);
                t = loop(
                    OTIO_NS::RationalTime(-1.0, 24.0),
                    timeRange,
                    &looped);
                FTK_ASSERT(OTIO_NS::RationalTime(23.0, 24.0) == t);
                FTK_ASSERT(looped);
            }
        }

        void UtilTest::_util()
        {
            {
                auto otioClip = new OTIO_NS::Clip;
                OTIO_NS::ErrorStatus errorStatus;
                auto otioTrack = new OTIO_NS::Track();
                otioTrack->append_child(otioClip, &errorStatus);
                if (OTIO_NS::is_error(errorStatus))
                {
                    throw std::runtime_error("Cannot append child");
                }
                auto otioStack = new OTIO_NS::Stack;
                otioStack->append_child(otioTrack, &errorStatus);
                if (OTIO_NS::is_error(errorStatus))
                {
                    throw std::runtime_error("Cannot append child");
                }
                OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline(new OTIO_NS::Timeline);
                otioTimeline->set_tracks(otioStack);
                FTK_ASSERT(otioStack == getRoot(otioClip));
                FTK_ASSERT(otioStack == getParent<OTIO_NS::Stack>(otioClip));
                FTK_ASSERT(otioTrack == getParent<OTIO_NS::Track>(otioClip));
            }
            {
                VideoData a;
                a.time = OTIO_NS::RationalTime(1.0, 24.0);
                VideoData b;
                b.time = OTIO_NS::RationalTime(1.0, 24.0);
                FTK_ASSERT(isTimeEqual(a, b));
            }
        }

        void UtilTest::_audio()
        {
            {
                audio::Info info(2, audio::DataType::S32, 48000);
                std::vector<AudioData> data;
                auto out = audioCopy(info, data, Playback::Forward, 0, 2000);
                FTK_ASSERT(out.empty());

                auto audio = audio::Audio::create(info, info.sampleRate);
                audio::S32_T* audioP = reinterpret_cast<audio::S32_T*>(audio->getData());
                for (size_t i = 0; i < info.sampleRate; ++i, audioP += 2)
                {
                    audioP[0] = i;
                    audioP[1] = i + 1;
                }
                data.push_back(AudioData({ 0.0, { { audio } } }));
                out = audioCopy(info, data, Playback::Forward, 0, 2000);
                FTK_ASSERT(1 == out.size());
                FTK_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                for (size_t i = 0; i < out[0]->getSampleCount(); ++i, audioP += 2)
                {
                    FTK_ASSERT(i == audioP[0]);
                    FTK_ASSERT((i + 1) == audioP[1]);
                }

                out = audioCopy(info, data, Playback::Forward, info.sampleRate - 1000, 2000);
                FTK_ASSERT(1 == out.size());
                FTK_ASSERT(1000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                for (size_t i = 0, j = info.sampleRate - 1000; i < out[0]->getSampleCount(); ++i, ++j, audioP += 2)
                {
                    FTK_ASSERT(j == audioP[0]);
                    FTK_ASSERT((j + 1) == audioP[1]);
                }

                data.push_back(AudioData({ 1.0, { { audio } } }));
                out = audioCopy(info, data, Playback::Forward, info.sampleRate - 1000, 2000);
                FTK_ASSERT(1 == out.size());
                FTK_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                size_t i = 0;
                size_t j = info.sampleRate - 1000;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    FTK_ASSERT(j == audioP[0]);
                    FTK_ASSERT((j + 1) == audioP[1]);
                }
                i = 0;
                j = 0;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    FTK_ASSERT(j == audioP[0]);
                    FTK_ASSERT((j + 1) == audioP[1]);
                }

                out = audioCopy(info, data, Playback::Reverse, info.sampleRate, 2000);
                FTK_ASSERT(1 == out.size());
                FTK_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                i = 0;
                j = info.sampleRate - 2000;
                for (; i < 2000; ++i, ++j, audioP += 2)
                {
                    FTK_ASSERT(j == audioP[0]);
                    FTK_ASSERT((j + 1) == audioP[1]);
                }

                out = audioCopy(info, data, Playback::Reverse, info.sampleRate + 1000, 2000);
                FTK_ASSERT(1 == out.size());
                FTK_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                i = 0;
                j = info.sampleRate - 1000;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    FTK_ASSERT(j == audioP[0]);
                    FTK_ASSERT((j + 1) == audioP[1]);
                }
                i = 0;
                j = 0;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    FTK_ASSERT(j == audioP[0]);
                    FTK_ASSERT((j + 1) == audioP[1]);
                }
            }
        }
        
        void UtilTest::_otioz()
        {
            std::vector<file::FileInfo> list;
            file::list(TLRENDER_SAMPLE_DATA, list);
            for (const auto& entry : list)
            {
                if (".otio" == entry.getPath().getExtension())
                {
                    OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> timeline(
                        dynamic_cast<OTIO_NS::Timeline*>(OTIO_NS::Timeline::from_json_file(entry.getPath().get())));
                    file::Path outputPath = entry.getPath();
                    outputPath.setExtension(".otioz");
                    writeOTIOZ(
                        outputPath.get(-1, file::PathType::FileName),
                        timeline,
                        TLRENDER_SAMPLE_DATA);
                }
            }
        }
    }
}
