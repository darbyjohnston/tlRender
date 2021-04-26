// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace render
    {
        inline size_t VBO::getSize() const
        {
            return _size;
        }

        inline VBOType VBO::getType() const
        {
            return _type;
        }

        inline GLuint VBO::getID() const
        {
            return _vbo;
        }

        inline GLuint VAO::getID() const
        {
            return _vao;
        }
    }
}
