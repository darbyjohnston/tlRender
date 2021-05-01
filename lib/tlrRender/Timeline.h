// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrRender/IO.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>

namespace tlr
{
    //! Timelines.
    namespace timeline
    {
        //! Playback modes.
        enum class Playback
        {
            Stop,
            Forward,
            Reverse,

            Count,
            First = Stop
        };
        TLR_ENUM_LABEL(Playback);

        //! Playback loop modes.
        enum class Loop
        {
            Loop,
            Once,
            PingPong,

            Count,
            First = Loop
        };
        TLR_ENUM_LABEL(Loop);

        //! Timeline.
        class Timeline : public std::enable_shared_from_this<Timeline>
        {
            TLR_NON_COPYABLE(Timeline);

        protected:
            void _init(const std::string& fileName);
            Timeline();

        public:
            ~Timeline();

            //! Create a new timeline.
            static std::shared_ptr<Timeline> create(const std::string& fileName);

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the image info (from the first clip in the timeline).
            const imaging::Info& getImageInfo() const;

            //! Get the current time.
            const otime::RationalTime& getCurrentTime() const;

            //! Get the playback mode.
            Playback getPlayback() const;

            //! Set the playback mode.
            void setPlayback(Playback);

            //! Get the playback loop mode.
            Loop getLoop() const;

            //! Set the playback loop mode.
            void setLoop(Loop);

            //! Seek.
            void seek(const otime::RationalTime&);

            //! Tick the timeline.
            void tick();

            //! Get the current image.
            const std::shared_ptr<imaging::Image>& getCurrentImage() const;

            //! Set the I/O video queue size.
            void setVideoQueueSize(size_t);

        private:
            otio::SerializableObject::Retainer<otio::Timeline> _timeline;
            otio::SerializableObject::Retainer<otio::Track> _flattenedTimeline;
            otime::RationalTime _duration;

            std::shared_ptr<io::System> _ioSystem;
            imaging::Info _imageInfo;
            typedef std::pair<otio::SerializableObject::Retainer<otio::Clip>, std::shared_ptr<io::IRead> > Reader;
            std::vector<Reader> _readers;

            std::chrono::steady_clock::time_point _startTime;
            otime::RationalTime _currentTime;
            Playback _playback = Playback::Stop;
            Loop _loop = Loop::Loop;
            otime::RationalTime _playbackStartTime;

            std::shared_ptr<imaging::Image> _currentImage;
        };
    }

    TLR_ENUM_SERIALIZE(timeline::Playback);
    TLR_ENUM_SERIALIZE(timeline::Loop);
}

#include <tlrRender/TimelineInline.h>
