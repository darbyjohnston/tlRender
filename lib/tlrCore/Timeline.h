// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/IO.h>
#include <tlrCore/Util.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>

namespace tlr
{
    //! Timelines.
    namespace timeline
    {
        //! Get the timeline file extensions.
        std::vector<std::string> getExtensions();

        //! Convert frames to ranges.
        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime>);

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

            //! \name Information
            ///@{

            //! Get the file name.
            const std::string& getFileName() const;

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the global start time.
            const otime::RationalTime& getGlobalStartTime() const;

            //! Get the image info.
            const imaging::Info& getImageInfo() const;

            //! Get the clip time ranges.
            const std::vector<otime::TimeRange>& getClipRanges() const;

            ///@}

            //! \name Frames
            ///@{

            //! Set the active time ranges. This informs the timeline which
            //! I/O readers to keep active.
            void setActiveRanges(const std::vector<otime::TimeRange>&);

            //! Render a frame.
            std::future<io::VideoFrame> render(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>& = nullptr);

            //! Cancel renders.
            void cancelRenders();

            ///@}

            //! Tick the timeline.
            void tick();

        private:
            std::string _fixFileName(const std::string&) const;
            std::string _getFileName(const otio::ImageSequenceReference*) const;
            std::string _getFileName(const otio::MediaReference*) const;
            otime::TimeRange _getRange(const otio::SerializableObject::Retainer<otio::Clip>&) const;

            std::string _fileName;
            otio::SerializableObject::Retainer<otio::Timeline> _timeline;
            otio::SerializableObject::Retainer<otio::Track> _flattenedTimeline;
            otime::RationalTime _duration = invalidTime;
            otime::RationalTime _globalStartTime = invalidTime;
            std::shared_ptr<io::System> _ioSystem;
            imaging::Info _imageInfo;
            std::vector<otio::Clip*> _clips;
            std::vector<otime::TimeRange> _clipRanges;
            struct Reader
            {
                std::shared_ptr<io::IRead> read;
                io::Info info;
            };
            std::map<otio::Clip*, Reader> _readers;
            std::list<std::shared_ptr<io::IRead> > _stoppedReaders;
            std::vector<otime::TimeRange> _activeRanges;
        };
    }
}

#include <tlrCore/TimelineInline.h>
