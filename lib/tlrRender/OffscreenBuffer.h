// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrAV/Image.h>

#include <glad.h>

#include <memory>

namespace tlr
{
    namespace render
    {
        //! Offscreen buffer depth types.
        enum class OffscreenDepthType
        {
            None,
            _24,
            _32,

            Count,
            First = None
        };

        //! Get the OpenGL depth buffer format.
        GLenum getFormat(OffscreenDepthType);

        //! Get the OpenGL depth buffer internal format.
        GLenum getInternalFormat(OffscreenDepthType);

        //! Get the OpenGL depth buffer type.
        GLenum getType(OffscreenDepthType);

        //! Offscreen multisampling.
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

        //! OpenGL offscreen buffer.
        class OffscreenBuffer : public std::enable_shared_from_this<OffscreenBuffer>
        {
            TLR_NON_COPYABLE(OffscreenBuffer);

         protected:
            void _init(
                const imaging::Size&,
                imaging::PixelType,
                OffscreenDepthType,
                OffscreenSampling);
            OffscreenBuffer();

        public:
            ~OffscreenBuffer();

            //! Create a new offscreen buffer.
            static std::shared_ptr<OffscreenBuffer> create(
                const imaging::Size&,
                imaging::PixelType);

            //! Create a new offscreen buffer.
            static std::shared_ptr<OffscreenBuffer> create(
                const imaging::Size&,
                imaging::PixelType,
                OffscreenDepthType,
                OffscreenSampling);

            //! Get the size.
            const imaging::Size& getSize() const;

            //! Get the color buffer pixel type.
            imaging::PixelType getColorType() const;

            //! Get the depth buffer type.
            OffscreenDepthType getDepthType() const;

            //! Get the multisampling value.
            OffscreenSampling getSampling() const;

            //! Get the offscreen buffer ID.
            GLuint getID() const;

            //! Get the color buffer ID.
            GLuint getColorID() const;

            //! Get the depth buffer ID.
            GLuint getDepthID() const;

            //! Bind the offscreen buffer.
            void bind();

        private:
            imaging::Size _size;
            imaging::PixelType _colorType = imaging::PixelType::None;
            OffscreenDepthType _depthType = OffscreenDepthType::None;
            OffscreenSampling _sampling = OffscreenSampling::None;
            GLuint _id = 0;
            GLuint _colorID = 0;
            GLuint _depthID = 0;
        };

        //! Offscreen Buffer Binding
        class OffscreenBufferBinding
        {
        public:
            explicit OffscreenBufferBinding(const std::shared_ptr<OffscreenBuffer>&);
            ~OffscreenBufferBinding();

        private:
            std::shared_ptr<OffscreenBuffer> _buffer;
        };
    }
}

#include <tlrRender/OffscreenBufferInline.h>

