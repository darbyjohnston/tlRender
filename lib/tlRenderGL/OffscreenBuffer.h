// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <tlGlad/gl.h>

#include <memory>

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
            core::imaging::PixelType colorType = core::imaging::PixelType::None;
            GLint colorMin = GL_LINEAR;
            GLint colorMag = GL_LINEAR;
            OffscreenDepth depth = OffscreenDepth::None;
            OffscreenStencil stencil = OffscreenStencil::None;
            OffscreenSampling sampling = OffscreenSampling::None;
        };

        //! Offscreen buffer.
        class OffscreenBuffer : public std::enable_shared_from_this<OffscreenBuffer>
        {
            TLRENDER_NON_COPYABLE(OffscreenBuffer);

        protected:
            void _init(
                const core::imaging::Size&,
                const OffscreenBufferOptions&);
            OffscreenBuffer();

        public:
            ~OffscreenBuffer();

            //! Create a new offscreen buffer.
            static std::shared_ptr<OffscreenBuffer> create(
                const core::imaging::Size&,
                const OffscreenBufferOptions&);

            //! Get the offscreen buffer size.
            const core::imaging::Size& getSize() const;

            //! Get the options.
            const OffscreenBufferOptions& getOptions() const;

            //! Get the offscreen buffer ID.
            GLuint getID() const;

            //! Get the color texture ID.
            GLuint getColorID() const;

            //! Bind the offscreen buffer.
            void bind();

        private:
            core::imaging::Size _size;
            OffscreenBufferOptions _options;
            GLuint _id = 0;
            GLuint _colorID = 0;
            GLuint _depthStencilID = 0;
        };

        //! Offscreen Buffer Binding
        class OffscreenBufferBinding
        {
        public:
            explicit OffscreenBufferBinding(const std::shared_ptr<OffscreenBuffer>&);

            ~OffscreenBufferBinding();

        private:
            std::shared_ptr<OffscreenBuffer> _buffer;
            GLint _previous = 0;
        };
    }
}
