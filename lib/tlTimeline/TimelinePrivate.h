// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <opentimelineio/clip.h>

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

namespace tl
{
    namespace timeline
    {
        struct Timeline::Private
        {
            file::Path getPath(const otio::MediaReference*) const;

            static std::vector<io::MemoryRead> getMemoryRead(const otio::MediaReference*);

            bool getVideoInfo(const otio::Composable*);
            bool getAudioInfo(const otio::Composable*);

            float transitionValue(double frame, double in, double out) const;

            void tick();
            void requests();
            struct Reader
            {
                std::shared_ptr<io::IRead> read;
                io::Info info;
                otime::TimeRange range;
            };
            Reader createReader(
                const otio::Track*,
                const otio::Clip*,
                const io::Options&);
            std::future<io::VideoData> readVideo(
                const otio::Track*,
                const otio::Clip*,
                const otime::RationalTime&,
                uint16_t videoLayer);
            std::future<io::AudioData> readAudio(
                const otio::Track*,
                const otio::Clip*,
                const otime::TimeRange&);
            void stopReaders();
            void delReaders();

            std::weak_ptr<system::Context> context;
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            file::Path path;
            file::Path audioPath;
            Options options;
            otime::RationalTime duration = time::invalidTime;
            otime::RationalTime globalStartTime = time::invalidTime;
            io::Info ioInfo;
            std::vector<otime::TimeRange> activeRanges;

            struct VideoLayerData
            {
                VideoLayerData() {};
                VideoLayerData(VideoLayerData&&) = default;

                std::future<io::VideoData> image;
                std::future<io::VideoData> imageB;
                Transition transition = Transition::None;
                float transitionValue = 0.F;
            };
            struct VideoRequest
            {
                VideoRequest() {};
                VideoRequest(VideoRequest&&) = default;

                otime::RationalTime time = time::invalidTime;
                uint16_t videoLayer = 0;
                std::promise<VideoData> promise;

                std::vector<VideoLayerData> layerData;
            };
            std::list<std::shared_ptr<VideoRequest> > videoRequests;
            std::list<std::shared_ptr<VideoRequest> > videoRequestsInProgress;

            struct AudioLayerData
            {
                AudioLayerData() {};
                AudioLayerData(AudioLayerData&&) = default;

                std::future<io::AudioData> audio;
                std::future<io::AudioData> audioB;
                float transitionValue = 0.F;
            };
            struct AudioRequest
            {
                AudioRequest() {};
                AudioRequest(AudioRequest&&) = default;

                int64_t seconds = -1;
                std::promise<AudioData> promise;

                std::vector<AudioLayerData> layerData;
            };
            std::list<std::shared_ptr<AudioRequest> > audioRequests;
            std::list<std::shared_ptr<AudioRequest> > audioRequestsInProgress;

            std::condition_variable requestCV;

            std::map<const otio::Clip*, Reader> readers;
            std::list<std::shared_ptr<io::IRead> > stoppedReaders;

            std::thread thread;
            std::mutex mutex;
            bool stopped = false;
            std::atomic<bool> running;

            std::chrono::steady_clock::time_point logTimer;
        };
    }
}
