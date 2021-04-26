// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace render
    {
        inline const imaging::Info& Texture::getInfo() const
        {
            return _info;
        }

        inline GLuint Texture::getID() const
        {
            return _id;
        }
    }
}
