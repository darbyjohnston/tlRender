// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Image.h>
#include <tlrCore/Time.h>

#include <opentimelineio/composable.h>

#include <future>

namespace tlr
{
    //! Timelines.
    namespace timeline
    {\
        //! Timeout for frame requests.
        const std::chrono::microseconds requestTimeout(1000);

        //! Get the timeline file extensions.
        std::vector<std::string> getExtensions();

        //! Convert frames to ranges.
        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime>);

        //! Get the ancestor (highest parent).
        const otio::Composable* getAncestor(const otio::Composable*);

        //! Transitions.
        enum class Transition
        {
            None,
            Dissolve,

            Count,
            First = None
        };
        TLR_ENUM(Transition);

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
            otime::RationalTime time = invalidTime;
            std::vector<FrameLayer> layers;

            bool operator == (const Frame&) const;
            bool operator != (const Frame&) const;
        };

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

            ///@}

            //! \name Frames
            ///@{

            //! Set the active time ranges. This informs the timeline which
            //! I/O readers to keep active.
            void setActiveRanges(const std::vector<otime::TimeRange>&);

            //! Get a frame.
            std::future<Frame> getFrame(const otime::RationalTime&);

            //! Cancel frames.
            void cancelFrames();

            ///@}

        private:
            TLR_PRIVATE();
        };
    }

    TLR_ENUM_SERIALIZE(timeline::Transition);
}

