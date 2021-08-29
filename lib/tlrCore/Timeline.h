// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Context.h>
#include <tlrCore/Image.h>
#include <tlrCore/Path.h>
#include <tlrCore/Time.h>

#include <opentimelineio/composable.h>
#include <opentimelineio/item.h>

#include <future>

namespace tlr
{
    //! Timelines.
    namespace timeline
    {
        //! Number of frame requests to handle.
        const size_t requestCount = 8;

        //! Timeout for frame requests.
        const std::chrono::milliseconds requestTimeout(1);

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
                const file::Path&,
                const std::shared_ptr<core::Context>&);
            Timeline();

        public:
            ~Timeline();

            //! Create a new timeline.
            static std::shared_ptr<Timeline> create(
                const file::Path&,
                const std::shared_ptr<core::Context>&);

            //! Get the context.
            const std::shared_ptr<core::Context>& getContext() const;

            //! Get the path.
            const file::Path& getPath() const;

            //! \name Information
            ///@{

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
            std::future<Frame> getFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>& = nullptr);

            //! Cancel frames.
            void cancelFrames();

            ///@}

        private:
            TLR_PRIVATE();
        };
    }
}

#include <tlrCore/TimelineInline.h>
