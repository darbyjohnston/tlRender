// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace render
    {
        inline const imaging::Size& OffscreenBuffer::getSize() const
        {
            return _size;
        }

        inline imaging::PixelType OffscreenBuffer::getColorType() const
        {
            return _colorType;
        }

        inline OffscreenDepthType OffscreenBuffer::getDepthType() const
        {
            return _depthType;
        }

        inline OffscreenSampling OffscreenBuffer::getSampling() const
        {
            return _sampling;
        }

        inline GLuint OffscreenBuffer::getID() const
        {
            return _id;
        }

        inline GLuint OffscreenBuffer::getColorID() const
        {
            return _colorID;
        }

        inline GLuint OffscreenBuffer::getDepthID() const
        {
            return _depthID;
        }

    }
}