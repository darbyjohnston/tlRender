// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/RenderOptions.h>

#include <tlCore/Context.h>
#include <tlCore/Time.h>

#include <opentimelineio/anyDictionary.h>

namespace tl
{
    namespace imaging
    {
        class FontSystem;
    }

    namespace timeline
    {
        class IRender;

        //! Base class for render primitives.
        class IPrimitive : public std::enable_shared_from_this<IPrimitive>
        {
            TLRENDER_NON_COPYABLE(IPrimitive);

        protected:
            void _init(
                const otio::AnyDictionary&,
                const std::shared_ptr<system::Context>&);
            IPrimitive();

        public:
            virtual ~IPrimitive() = 0;

            virtual void render(
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<IRender>&) = 0;

        protected:
            std::weak_ptr<system::Context> _context;
        };

        //! Rectangle primitive.
        class RectPrimitive : public IPrimitive
        {
        protected:
            void _init(
                const otio::AnyDictionary&,
                const std::shared_ptr<system::Context>&);
            RectPrimitive();

        public:
            ~RectPrimitive() override;

            static std::shared_ptr<RectPrimitive> create(
                const otio::AnyDictionary&,
                const std::shared_ptr<system::Context>&);

            void render(
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<IRender>&) override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Text primitive.
        class TextPrimitive : public IPrimitive
        {
        protected:
            void _init(
                const otio::AnyDictionary&,
                const std::shared_ptr<system::Context>&);
            TextPrimitive();

        public:
            ~TextPrimitive() override;

            static std::shared_ptr<TextPrimitive> create(
                const otio::AnyDictionary&,
                const std::shared_ptr<system::Context>&);

            void render(
                const std::shared_ptr<imaging::FontSystem>&,
                const std::shared_ptr<IRender>&) override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Primitive factory.
        class PrimitiveFactory : public std::enable_shared_from_this<PrimitiveFactory>
        {
            TLRENDER_NON_COPYABLE(PrimitiveFactory);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            PrimitiveFactory();

        public:
            static std::shared_ptr<PrimitiveFactory> create(const std::shared_ptr<system::Context>&);

            void read(
                const otio::AnyDictionary&,
                std::vector<std::shared_ptr<IPrimitive> >&);

        private:
            TLRENDER_PRIVATE();
        };

        //! Transitions.
        enum class Transition
        {
            None,
            Dissolve,

            Count,
            First = None
        };
        TLRENDER_ENUM(Transition);
        TLRENDER_ENUM_SERIALIZE(Transition);

        //! Convert to a transition.
        Transition toTransition(const std::string&);

        //! Video layer.
        struct VideoLayer
        {
            std::shared_ptr<imaging::Image> image;
            ImageOptions imageOptions;
            std::vector<std::shared_ptr<IPrimitive> > primitives;

            std::shared_ptr<imaging::Image> imageB;
            ImageOptions imageOptionsB;
            std::vector<std::shared_ptr<IPrimitive> > primitivesB;

            Transition transition = Transition::None;
            float transitionValue = 0.F;

            bool operator == (const VideoLayer&) const;
            bool operator != (const VideoLayer&) const;
        };

        //! Video data.
        struct VideoData
        {
            otime::RationalTime time = time::invalidTime;
            std::vector<VideoLayer> layers;
            DisplayOptions displayOptions;

            bool operator == (const VideoData&) const;
            bool operator != (const VideoData&) const;
        };

        //! Compare the time values of video data.
        bool isTimeEqual(const VideoData&, const VideoData&);
    }
}

#include <tlTimeline/VideoInline.h>
