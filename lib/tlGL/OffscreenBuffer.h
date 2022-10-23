// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace gl
    {
        //! Offscreen buffer depth size.
        enum class OffscreenDepth
        {
            None,
            _24,
            _32,

            Count,
            First = None
        };

        //! Offscreen buffer stencil size.
        enum class OffscreenStencil
        {
            None,
            _8,

            Count,
            First = None
        };

        //! Offscreen buffer multisampling.
        enum class OffscreenSampling
        {
            None,
            _2,
            _4,
            _8,
            _16,

            Count,
            First = None
        };

        //! Offscreen buffer options.
        struct OffscreenBufferOptions
        {
            imaging::PixelType colorType = imaging::PixelType::None;
            timeline::ImageFilters colorFilters;
            OffscreenDepth depth = OffscreenDepth::None;
            OffscreenStencil stencil = OffscreenStencil::None;
            OffscreenSampling sampling = OffscreenSampling::None;

            bool operator == (const OffscreenBufferOptions&) const;
            bool operator != (const OffscreenBufferOptions&) const;
        };

        //! Offscreen buffer.
        class OffscreenBuffer : public std::enable_shared_from_this<OffscreenBuffer>
        {
            TLRENDER_NON_COPYABLE(OffscreenBuffer);

        protected:
            void _init(
                const imaging::Size&,
                const OffscreenBufferOptions&);

            OffscreenBuffer();

        public:
            ~OffscreenBuffer();

            //! Create a new offscreen buffer.
            static std::shared_ptr<OffscreenBuffer> create(
                const imaging::Size&,
                const OffscreenBufferOptions&);

            //! Get the offscreen buffer size.
            const imaging::Size& getSize() const;

            //! Get the options.
            const OffscreenBufferOptions& getOptions() const;

            //! Get the offscreen buffer ID.
            unsigned int getID() const;

            //! Get the color texture ID.
            unsigned int getColorID() const;

            //! Bind the offscreen buffer.
            void bind();

        private:
            TLRENDER_PRIVATE();
        };

        //! Check whether the offscreen buffer should be created or re-created.
        bool doCreate(
            const std::shared_ptr<OffscreenBuffer>&,
            const imaging::Size&,
            const OffscreenBufferOptions&);

        //! Offscreen Buffer Binding
        class OffscreenBufferBinding
        {
        public:
            explicit OffscreenBufferBinding(const std::shared_ptr<OffscreenBuffer>&);

            ~OffscreenBufferBinding();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
