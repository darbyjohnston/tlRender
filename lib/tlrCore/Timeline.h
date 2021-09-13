// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AVIO.h>
#include <tlrCore/Context.h>

#include <opentimelineio/composable.h>
#include <opentimelineio/item.h>
#include <opentimelineio/timeline.h>

#include <future>

namespace tlr
{
    //! Timelines.
    namespace timeline
    {
        //! Get the timeline file extensions.
        std::vector<std::string> getExtensions();

        //! Convert frames to ranges.
        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime>);

        //! Get the root (highest parent).
        const otio::Composable* getRoot(const otio::Composable*);

        //! Get the parent of the given type.
        template<typename T>
        const T* getParent(const otio::Item*);

        //! Transitions.
        enum class Transition
        {
            None,
            Dissolve,

            Count,
            First = None
        };
        TLR_ENUM(Transition);
        TLR_ENUM_SERIALIZE(Transition);

        //! Convert to a transition.
        Transition toTransition(const std::string&);

        //! Frame layer.
        struct FrameLayer
        {
            std::shared_ptr<imaging::Image> image;
            std::shared_ptr<imaging::Image> imageB;
            Transition transition = Transition::None;
            float transitionValue = 0.F;

            bool operator == (const FrameLayer&) const;
            bool operator != (const FrameLayer&) const;
        };

        //! Frame.
        struct Frame
        {
            otime::RationalTime time = time::invalidTime;
            std::vector<FrameLayer> layers;

            bool operator == (const Frame&) const;
            bool operator != (const Frame&) const;
        };

        //! Compare the time value of two frames.
        bool isTimeEqual(const Frame&, const Frame&);

        //! Timeline.
        class Timeline : public std::enable_shared_from_this<Timeline>
        {
            TLR_NON_COPYABLE(Timeline);

        protected:
            void _init(
                const otio::SerializableObject::Retainer<otio::Timeline>&,
                const std::shared_ptr<core::Context>&);
            Timeline();

        public:
            ~Timeline();

            //! Create a new timeline.
            static std::shared_ptr<Timeline> create(
                const otio::SerializableObject::Retainer<otio::Timeline>&,
                const std::shared_ptr<core::Context>&);

            //! Create a new timeline from a file path. The file path can point
            //! to an .otio file, movie file, or image sequence.
            static std::shared_ptr<Timeline> create(
                const file::Path&,
                const std::shared_ptr<core::Context>&);

            //! Get the context.
            const std::weak_ptr<core::Context>& getContext() const;

            //! Get the timeline.
            const otio::SerializableObject::Retainer<otio::Timeline>& getTimeline() const;

            //! Get the path.
            const file::Path& getPath() const;

            //! \name Information
            ///@{

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the global start time.
            const otime::RationalTime& getGlobalStartTime() const;

            //! Get the video information. This information is retreived from
            //! the first clip in the timeline. The vector represents the
            //! different layers that are available.
            const std::vector<imaging::Info>& getVideoInfo() const;

            ///@}

            //! \name Frames
            ///@{

            //! Set the active time ranges. This informs the timeline which
            //! I/O readers to keep active.
            void setActiveRanges(const std::vector<otime::TimeRange>&);

            //! Get a frame.
            std::future<Frame> getFrame(
                const otime::RationalTime&,
                uint16_t layer = 0,
                const std::shared_ptr<imaging::Image>& = nullptr);

            //! Cancel frames.
            void cancelFrames();

            ///@}

            //! \name Options
            ///@{

            //! Get the number of frame requests.
            size_t getRequestCount() const;

            //! Set the number of frame requests.
            void setRequestCount(size_t);

            //! Get the frame request timeout.
            std::chrono::milliseconds getRequestTimeout() const;

            //! Set the frame request timeout.
            void setRequestTimeout(const std::chrono::milliseconds&);

            //! Set the I/O options.
            void setIOOptions(const avio::Options&);

            ///@}

        private:
            TLR_PRIVATE();
        };
    }
}

#include <tlrCore/TimelineInline.h>
