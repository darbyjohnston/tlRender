// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrRender/Mesh.h>

#include <tlrCore/Math.h>

#include <array>

namespace tlr
{
    namespace render
    {
        std::size_t getByteCount(VBOType value)
        {
            const std::array<size_t, static_cast<size_t>(VBOType::Count)> data =
            {
                12, // 2 * sizeof(float) + 2 * sizeof(uint16_t)
                12, // 3 * sizeof(float)
                16, // 3 * sizeof(float) + 2 * sizeof(uint16_t)
                20, // 3 * sizeof(float) + 2 * sizeof(uint16_t) + sizeof(PackedNormal)
                24, // 3 * sizeof(float) + 2 * sizeof(uint16_t) + sizeof(PackedNormal) + sizeof(PackedColor)
                32, // 3 * sizeof(float) + 2 * sizeof(float) + 3 * sizeof(float)
                44, // 3 * sizeof(float) + 2 * sizeof(float) + 3 * sizeof(float) + 3 * sizeof(float)
                16  // 3 * sizeof(float) + sizeof(PackedColor)
            };
            return data[static_cast<size_t>(value)];
        }

        void VBO::_init(std::size_t size, VBOType type)
        {
            _size = size;
            _type = type;
            glGenBuffers(1, &_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(_size * getByteCount(type)), NULL, GL_DYNAMIC_DRAW);
        }

        VBO::VBO()
        {}

        VBO::~VBO()
        {
            if (_vbo)
            {
                glDeleteBuffers(1, &_vbo);
                _vbo = 0;
            }
        }

        std::shared_ptr<VBO> VBO::create(std::size_t size, VBOType type)
        {
            auto out = std::shared_ptr<VBO>(new VBO);
            out->_init(size, type);
            return out;
        }

        void VBO::copy(const std::vector<uint8_t>& data)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizei>(data.size()), (void*)data.data());
        }

        void VBO::copy(const std::vector<uint8_t>& data, std::size_t offset)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferSubData(GL_ARRAY_BUFFER, offset, static_cast<GLsizei>(data.size()), (void*)data.data());
        }

        void VBO::copy(const std::vector<uint8_t>& data, std::size_t offset, std::size_t size)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferSubData(GL_ARRAY_BUFFER, offset, static_cast<GLsizei>(size), (void*)data.data());
        }
        
        namespace
        {
            struct PackedNormal
            {
                unsigned int x : 10;
                unsigned int y : 10;
                unsigned int z : 10;
                unsigned int unused : 2;
            };

            struct PackedColor
            {
                unsigned int r : 8;
                unsigned int g : 8;
                unsigned int b : 8;
                unsigned int a : 8;
            };
        }

        void VAO::_init(VBOType type, GLuint vbo)
        {
            glGenVertexArrays(1, &_vao);
            glBindVertexArray(_vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            const std::size_t byteCount = getByteCount(type);
            switch (type)
            {
            case VBOType::Pos2_F32_UV_U16:
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, static_cast<GLsizei>(byteCount), (GLvoid*)8);
                glEnableVertexAttribArray(1);
                break;
            case VBOType::Pos3_F32:
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                break;
            case VBOType::Pos3_F32_UV_U16:
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, static_cast<GLsizei>(byteCount), (GLvoid*)12);
                glEnableVertexAttribArray(1);
                break;
            case VBOType::Pos3_F32_UV_F32_Normal_F32:
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)12);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)20);
                glEnableVertexAttribArray(2);
                break;
            case VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32:
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)12);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)20);
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)32);
                glEnableVertexAttribArray(3);
                break;
            case VBOType::Pos3_F32_Color_U8:
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, static_cast<GLsizei>(byteCount), (GLvoid*)12);
                glEnableVertexAttribArray(1);
                break;
            default: break;
            }
        }

        VAO::VAO()
        {}

        VAO::~VAO()
        {
            if (_vao)
            {
                glDeleteVertexArrays(1, &_vao);
                _vao = 0;
            }
        }

        std::shared_ptr<VAO> VAO::create(VBOType type, GLuint vbo)
        {
            auto out = std::shared_ptr<VAO>(new VAO);
            out->_init(type, vbo);
            return out;
        }

        void VAO::bind()
        {
            glBindVertexArray(_vao);
        }

        void VAO::draw(GLenum mode, std::size_t offset, std::size_t size)
        {
            glDrawArrays(mode, static_cast<GLsizei>(offset), static_cast<GLsizei>(size));
        }
    }
}
