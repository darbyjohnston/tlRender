// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Range.h>
#include <tlrCore/Util.h>

#include <glad/gl.h>

#include <memory>
#include <vector>

namespace tlr
{
    namespace geom
    {
        struct TriangleMesh;
    }

    namespace gl
    {
        //! Vertex buffer object types.
        enum class VBOType
        {
            Pos2_F32_UV_U16,
            Pos3_F32,
            Pos3_F32_UV_U16,
            Pos3_F32_UV_U16_Normal_U10,
            Pos3_F32_UV_U16_Normal_U10_Color_U8,
            Pos3_F32_UV_F32_Normal_F32,
            Pos3_F32_UV_F32_Normal_F32_Color_F32,
            Pos3_F32_Color_U8,

            Count,
            First = Pos2_F32_UV_U16
        };

        //! Get the number of bytes used to store vertex buffer object types.
        std::size_t getByteCount(VBOType);

        //! Convert a triangle mesh to vertex buffer data.
        std::vector<uint8_t> convert(
            const geom::TriangleMesh& mesh,
            gl::VBOType               type,
            const math::SizeTRange& range);

        //! OpenGL vertex buffer object.
        class VBO : public std::enable_shared_from_this<VBO>
        {
            TLR_NON_COPYABLE(VBO);

        protected:
            void _init(std::size_t size, VBOType);
            VBO();

        public:
            ~VBO();

            //! Create a new vertex buffer object.
            static std::shared_ptr<VBO> create(std::size_t size, VBOType);

            //! Get the size.
            std::size_t getSize() const;

            //! Get the type.
            VBOType getType() const;

            //! Get the OpenGL ID.
            GLuint getID() const;

            //! \name Copy
            //! Copy data to the vertex buffer object.
            ///@{

            void copy(const std::vector<uint8_t>&);
            void copy(const std::vector<uint8_t>&, std::size_t offset);
            void copy(const std::vector<uint8_t>&, std::size_t offset, std::size_t size);

            ///@}

        private:
            std::size_t _size = 0;
            VBOType _type = VBOType::First;
            GLuint _vbo = 0;
        };

        //! OpenGL vertex array object.
        class VAO : public std::enable_shared_from_this<VAO>
        {
            TLR_NON_COPYABLE(VAO);

        protected:
            void _init(VBOType, GLuint vbo);
            VAO();

        public:
            ~VAO();

            //! Create a new vertex array object.
            static std::shared_ptr<VAO> create(VBOType, GLuint vbo);

            //! Get the OpenGL ID.
            GLuint getID() const;

            //! Bind the vertex array object.
            void bind();

            //! Draw the vertex array object.
            void draw(GLenum mode, std::size_t offset, std::size_t size);

        private:
            GLuint _vao = 0;
        };
    }
}
