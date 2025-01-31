// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/UtilTest.h>

#include <tlTimeline/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/FileInfo.h>

#include <dtk/core/Format.h>

#include <opentimelineio/clip.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        UtilTest::UtilTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::UtilTest", context)
        {}

        std::shared_ptr<UtilTest> UtilTest::create(const std::shared_ptr<system::Context>& context)
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
                static_cast<int>(io::FileType::Movie) |
                static_cast<int>(io::FileType::Sequence) |
                static_cast<int>(io::FileType::Audio),
                _context))
            {
                std::stringstream ss;
                ss << "Timeline extension: " << i;
                _print(ss.str());
            }
            for (const auto& path : getPaths(
                file::Path(TLRENDER_SAMPLE_DATA),
                file::PathOptions(),
                _context))
            {
                _print(dtk::Format("Path: {0}").arg(path.get()));
            }
        }

        void UtilTest::_ranges()
        {
            {
                std::vector<otime::RationalTime> f;
                auto r = toRanges(f);
                TLRENDER_ASSERT(r.empty());
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24)
                };
                auto r = toRanges(f);
                TLRENDER_ASSERT(1 == r.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(1, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24)
                };
                auto r = toRanges(f);
                TLRENDER_ASSERT(1 == r.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(2, 24)
                };
                auto r = toRanges(f);
                TLRENDER_ASSERT(1 == r.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(3, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(2, 24)
                };
                auto r = toRanges(f);
                TLRENDER_ASSERT(2 == r.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(1, 24)) == r[0]);
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(2, 24), otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24)
                };
                auto r = toRanges(f);
                TLRENDER_ASSERT(2 == r.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(3, 24), otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24),
                    otime::RationalTime(4, 24)
                };
                auto r = toRanges(f);
                TLRENDER_ASSERT(2 == r.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(3, 24), otime::RationalTime(2, 24)) == r[1]);
            }
        }

        void UtilTest::_loop()
        {
            {
                const otio::TimeRange timeRange(
                    otio::RationalTime(0.0, 24.0),
                    otio::RationalTime(24.0, 24.0));
                bool looped = false;
                otio::RationalTime t = loop(
                    otio::RationalTime(0.0, 24.0),
                    timeRange,
                    &looped);
                TLRENDER_ASSERT(otio::RationalTime(0.0, 24.0) == t);
                TLRENDER_ASSERT(!looped);
                t = loop(
                    otio::RationalTime(24.0, 24.0),
                    timeRange,
                    &looped);
                TLRENDER_ASSERT(otio::RationalTime(0.0, 24.0) == t);
                TLRENDER_ASSERT(looped);
                t = loop(
                    otio::RationalTime(-1.0, 24.0),
                    timeRange,
                    &looped);
                TLRENDER_ASSERT(otio::RationalTime(23.0, 24.0) == t);
                TLRENDER_ASSERT(looped);
            }
            {
                const otio::TimeRange timeRange(
                    otio::RationalTime(0.0, 24.0),
                    otio::RationalTime(24.0, 24.0));
                otio::TimeRange cacheRange(
                    otio::RationalTime(0.0, 24.0),
                    otio::RationalTime(12.0, 24.0));
                auto result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Forward);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    cacheRange }));

                cacheRange = otio::TimeRange(
                    otio::RationalTime(-1.0, 24.0),
                    otio::RationalTime(12.0, 24.0));
                result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Forward);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    otio::TimeRange(
                        otio::RationalTime(0.0, 24.0),
                        otio::RationalTime(11.0, 24.0)),
                    otio::TimeRange(
                        otio::RationalTime(23.0, 24.0),
                        otio::RationalTime(1.0, 24.0)) }));

                cacheRange = otio::TimeRange(
                    otio::RationalTime(13.0, 24.0),
                    otio::RationalTime(12.0, 24.0));
                result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Forward);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    otio::TimeRange(
                        otio::RationalTime(13.0, 24.0),
                        otio::RationalTime(11.0, 24.0)),
                    otio::TimeRange(
                        otio::RationalTime(0.0, 24.0),
                        otio::RationalTime(1.0, 24.0)) }));

                cacheRange = otio::TimeRange(
                    otio::RationalTime(-1.0, 24.0),
                    otio::RationalTime(26.0, 24.0));
                result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Forward);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    timeRange }));
            }
            {
                const otio::TimeRange timeRange(
                    otio::RationalTime(0.0, 24.0),
                    otio::RationalTime(24.0, 24.0));
                otio::TimeRange cacheRange(
                    otio::RationalTime(12.0, 24.0),
                    otio::RationalTime(12.0, 24.0));
                auto result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    cacheRange }));

                cacheRange = otio::TimeRange(
                    otio::RationalTime(13.0, 24.0),
                    otio::RationalTime(12.0, 24.0));
                result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    otio::TimeRange(
                        otio::RationalTime(0.0, 24.0),
                        otio::RationalTime(1.0, 24.0)),
                    otio::TimeRange(
                        otio::RationalTime(13.0, 24.0),
                        otio::RationalTime(11.0, 24.0)) }));

                cacheRange = otio::TimeRange(
                    otio::RationalTime(-1.0, 24.0),
                    otio::RationalTime(12.0, 24.0));
                result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    otio::TimeRange(
                        otio::RationalTime(0.0, 24.0),
                        otio::RationalTime(11.0, 24.0)),
                    otio::TimeRange(
                        otio::RationalTime(23.0, 24.0),
                        otio::RationalTime(1.0, 24.0)) }));

                cacheRange = otio::TimeRange(
                    otio::RationalTime(-1.0, 24.0),
                    otio::RationalTime(26.0, 24.0));
                result = loopCache(
                    cacheRange,
                    timeRange,
                    CacheDirection::Reverse);
                TLRENDER_ASSERT(result == std::vector<otime::TimeRange>({
                    timeRange }));
            }
        }

        void UtilTest::_util()
        {
            {
                auto otioClip = new otio::Clip;
                otio::ErrorStatus errorStatus;
                auto otioTrack = new otio::Track();
                otioTrack->append_child(otioClip, &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    throw std::runtime_error("Cannot append child");
                }
                auto otioStack = new otio::Stack;
                otioStack->append_child(otioTrack, &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    throw std::runtime_error("Cannot append child");
                }
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                otioTimeline->set_tracks(otioStack);
                TLRENDER_ASSERT(otioStack == getRoot(otioClip));
                TLRENDER_ASSERT(otioStack == getParent<otio::Stack>(otioClip));
                TLRENDER_ASSERT(otioTrack == getParent<otio::Track>(otioClip));
            }
            {
                VideoData a;
                a.time = otime::RationalTime(1.0, 24.0);
                VideoData b;
                b.time = otime::RationalTime(1.0, 24.0);
                TLRENDER_ASSERT(isTimeEqual(a, b));
            }
        }

        void UtilTest::_audio()
        {
            {
                audio::Info info(2, audio::DataType::S32, 48000);
                std::vector<AudioData> data;
                auto out = audioCopy(info, data, Playback::Forward, 0, 2000);
                TLRENDER_ASSERT(out.empty());

                auto audio = audio::Audio::create(info, info.sampleRate);
                audio::S32_T* audioP = reinterpret_cast<audio::S32_T*>(audio->getData());
                for (size_t i = 0; i < info.sampleRate; ++i, audioP += 2)
                {
                    audioP[0] = i;
                    audioP[1] = i + 1;
                }
                data.push_back(AudioData({ 0.0, { { audio } } }));
                out = audioCopy(info, data, Playback::Forward, 0, 2000);
                TLRENDER_ASSERT(1 == out.size());
                TLRENDER_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                for (size_t i = 0; i < out[0]->getSampleCount(); ++i, audioP += 2)
                {
                    TLRENDER_ASSERT(i == audioP[0]);
                    TLRENDER_ASSERT((i + 1) == audioP[1]);
                }

                out = audioCopy(info, data, Playback::Forward, info.sampleRate - 1000, 2000);
                TLRENDER_ASSERT(1 == out.size());
                TLRENDER_ASSERT(1000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                for (size_t i = 0, j = info.sampleRate - 1000; i < out[0]->getSampleCount(); ++i, ++j, audioP += 2)
                {
                    TLRENDER_ASSERT(j == audioP[0]);
                    TLRENDER_ASSERT((j + 1) == audioP[1]);
                }

                data.push_back(AudioData({ 1.0, { { audio } } }));
                out = audioCopy(info, data, Playback::Forward, info.sampleRate - 1000, 2000);
                TLRENDER_ASSERT(1 == out.size());
                TLRENDER_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                size_t i = 0;
                size_t j = info.sampleRate - 1000;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    TLRENDER_ASSERT(j == audioP[0]);
                    TLRENDER_ASSERT((j + 1) == audioP[1]);
                }
                i = 0;
                j = 0;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    TLRENDER_ASSERT(j == audioP[0]);
                    TLRENDER_ASSERT((j + 1) == audioP[1]);
                }

                out = audioCopy(info, data, Playback::Reverse, info.sampleRate, 2000);
                TLRENDER_ASSERT(1 == out.size());
                TLRENDER_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                i = 0;
                j = info.sampleRate - 2000;
                for (; i < 2000; ++i, ++j, audioP += 2)
                {
                    TLRENDER_ASSERT(j == audioP[0]);
                    TLRENDER_ASSERT((j + 1) == audioP[1]);
                }

                out = audioCopy(info, data, Playback::Reverse, info.sampleRate + 1000, 2000);
                TLRENDER_ASSERT(1 == out.size());
                TLRENDER_ASSERT(2000 == out[0]->getSampleCount());
                audioP = reinterpret_cast<audio::S32_T*>(out[0]->getData());
                i = 0;
                j = info.sampleRate - 1000;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    TLRENDER_ASSERT(j == audioP[0]);
                    TLRENDER_ASSERT((j + 1) == audioP[1]);
                }
                i = 0;
                j = 0;
                for (; i < 1000; ++i, ++j, audioP += 2)
                {
                    TLRENDER_ASSERT(j == audioP[0]);
                    TLRENDER_ASSERT((j + 1) == audioP[1]);
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
                    otio::SerializableObject::Retainer<otio::Timeline> timeline(
                        dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_file(entry.getPath().get())));
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
