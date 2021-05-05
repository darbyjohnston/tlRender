// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/BBox.h>
#include <tlrCore/Image.h>
#include <tlrCore/Util.h>
#include <tlrCore/ValueObserver.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>

namespace tlr
{
    namespace io
    {
        class IRead;
        class System;
    }

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

        //! Get the timeline file extensions.
        std::vector<std::string> getExtensions();

        //! Fit an image within a window.
        math::BBox2f fitWindow(const imaging::Size& image, const imaging::Size& window);

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

            //! Observe the current time.
            std::shared_ptr<Observer::IValueSubject<otime::RationalTime> > observeCurrentTime() const;

            //! Observe the playback mode.
            std::shared_ptr<Observer::IValueSubject<Playback> > observePlayback() const;

            //! Set the playback mode.
            void setPlayback(Playback);

            //! Observe the playback loop mode.
            std::shared_ptr<Observer::IValueSubject<Loop> > observeLoop() const;

            //! Set the playback loop mode.
            void setLoop(Loop);

            //! Seek.
            void seek(const otime::RationalTime&);

            //! Tick the timeline.
            void tick();

            //! Observe the current image.
            std::shared_ptr<Observer::IValueSubject<std::shared_ptr<imaging::Image> > > observeCurrentImage() const;

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

            std::shared_ptr<Observer::ValueSubject<otime::RationalTime> > _currentTime;
            std::shared_ptr<Observer::ValueSubject<Playback> > _playback;
            std::shared_ptr<Observer::ValueSubject<Loop> > _loop;
            std::shared_ptr<Observer::ValueSubject<std::shared_ptr<imaging::Image> > > _currentImage;

            std::chrono::steady_clock::time_point _startTime;
            otime::RationalTime _playbackStartTime;
        };
    }

    TLR_ENUM_SERIALIZE(timeline::Playback);
    TLR_ENUM_SERIALIZE(timeline::Loop);
}

#include <tlrCore/TimelineInline.h>
