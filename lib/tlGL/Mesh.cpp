// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/Mesh.h>

#include <tlCore/Math.h>
#include <tlCore/Mesh.h>

#include <array>

namespace tl
{
    namespace gl
    {
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

        std::size_t getByteCount(VBOType value)
        {
            const std::array<size_t, static_cast<size_t>(VBOType::Count)> data =
            {
                8,  // 2 * sizeof(float)
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

        std::vector<uint8_t> convert(
            const geom::TriangleMesh2& mesh,
            gl::VBOType type)
        {
            return convert(
                mesh,
                type,
                math::SizeTRange(0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1) : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh2& mesh,
            gl::VBOType type,
            const math::SizeTRange& range)
        {
            const size_t vertexByteCount = gl::getByteCount(type);
            std::vector<uint8_t> out((range.getMax() - range.getMin() + 1) * 3 * vertexByteCount);
            uint8_t* p = out.data();
            switch (type)
            {
            case gl::VBOType::Pos2_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex2* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        p += 2 * sizeof(float);
                    }
                }
                break;
            case gl::VBOType::Pos2_F32_UV_U16:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex2* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        p += 2 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].x * 65535.F), 0, 65535) : 0;
                        pu16[1] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].y * 65535.F), 0, 65535) : 0;
                        p += 2 * sizeof(uint16_t);
                    }
                }
                break;
            default: break;
            }
            return out;
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh3& mesh,
            gl::VBOType type)
        {
            return convert(
                mesh,
                type,
                math::SizeTRange(0, mesh.triangles.size() > 0 ? (mesh.triangles.size() - 1) : 0));
        }

        std::vector<uint8_t> convert(
            const geom::TriangleMesh3& mesh,
            gl::VBOType type,
            const math::SizeTRange& range)
        {
            const size_t vertexByteCount = gl::getByteCount(type);
            std::vector<uint8_t> out((range.getMax() - range.getMin() + 1) * 3 * vertexByteCount);
            uint8_t* p = out.data();
            switch (type)
            {
            case gl::VBOType::Pos3_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);
                    }
                }
                break;
            case gl::VBOType::Pos3_F32_UV_U16:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].x * 65535.F), 0, 65535) : 0;
                        pu16[1] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].y * 65535.F), 0, 65535) : 0;
                        p += 2 * sizeof(uint16_t);
                    }
                }
                break;
            case gl::VBOType::Pos3_F32_UV_U16_Normal_U10:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].x * 65535.F), 0, 65535) : 0;
                        pu16[1] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].y * 65535.F), 0, 65535) : 0;
                        p += 2 * sizeof(uint16_t);

                        const size_t n = vertices[k]->n;
                        auto packedNormal = reinterpret_cast<PackedNormal*>(p);
                        packedNormal->x = n ? math::clamp(static_cast<int>(mesh.n[n - 1].x * 511.F), -512, 511) : 0;
                        packedNormal->y = n ? math::clamp(static_cast<int>(mesh.n[n - 1].y * 511.F), -512, 511) : 0;
                        packedNormal->z = n ? math::clamp(static_cast<int>(mesh.n[n - 1].z * 511.F), -512, 511) : 0;
                        p += sizeof(PackedNormal);
                    }
                }
                break;
            case gl::VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        uint16_t* pu16 = reinterpret_cast<uint16_t*>(p);
                        pu16[0] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].x * 65535.F), 0, 65535) : 0;
                        pu16[1] = t ? math::clamp(static_cast<int>(mesh.t[t - 1].y * 65535.F), 0, 65535) : 0;
                        p += 2 * sizeof(uint16_t);

                        const size_t n = vertices[k]->n;
                        auto packedNormal = reinterpret_cast<PackedNormal*>(p);
                        packedNormal->x = n ? math::clamp(static_cast<int>(mesh.n[n - 1].x * 511.F), -512, 511) : 0;
                        packedNormal->y = n ? math::clamp(static_cast<int>(mesh.n[n - 1].y * 511.F), -512, 511) : 0;
                        packedNormal->z = n ? math::clamp(static_cast<int>(mesh.n[n - 1].z * 511.F), -512, 511) : 0;
                        p += sizeof(PackedNormal);

                        auto packedColor = reinterpret_cast<PackedColor*>(p);
                        packedColor->r = v ? math::clamp(static_cast<int>(mesh.c[v - 1].x * 255.F), 0, 255) : 0;
                        packedColor->g = v ? math::clamp(static_cast<int>(mesh.c[v - 1].y * 255.F), 0, 255) : 0;
                        packedColor->b = v ? math::clamp(static_cast<int>(mesh.c[v - 1].z * 255.F), 0, 255) : 0;
                        packedColor->a = 255;
                        p += sizeof(PackedColor);
                    }
                }
                break;
            case gl::VBOType::Pos3_F32_UV_F32_Normal_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = t ? mesh.t[t - 1].x : 0.F;
                        pf[1] = t ? mesh.t[t - 1].y : 0.F;
                        p += 2 * sizeof(float);

                        const size_t n = vertices[k]->n;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = n ? mesh.n[n - 1].x : 0.F;
                        pf[1] = n ? mesh.n[n - 1].y : 0.F;
                        pf[2] = n ? mesh.n[n - 1].z : 0.F;
                        p += 3 * sizeof(float);
                    }
                }
                break;
            case gl::VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32:
                for (size_t i = range.getMin(); i <= range.getMax(); ++i)
                {
                    const geom::Vertex3* vertices[] =
                    {
                        &mesh.triangles[i].v[0],
                        &mesh.triangles[i].v[1],
                        &mesh.triangles[i].v[2]
                    };
                    for (size_t k = 0; k < 3; ++k)
                    {
                        const size_t v = vertices[k]->v;
                        float* pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.v[v - 1].x : 0.F;
                        pf[1] = v ? mesh.v[v - 1].y : 0.F;
                        pf[2] = v ? mesh.v[v - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        const size_t t = vertices[k]->t;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = t ? mesh.t[t - 1].x : 0.F;
                        pf[1] = t ? mesh.t[t - 1].y : 0.F;
                        p += 2 * sizeof(float);

                        const size_t n = vertices[k]->n;
                        pf = reinterpret_cast<float*>(p);
                        pf[0] = n ? mesh.n[n - 1].x : 0.F;
                        pf[1] = n ? mesh.n[n - 1].y : 0.F;
                        pf[2] = n ? mesh.n[n - 1].z : 0.F;
                        p += 3 * sizeof(float);

                        pf = reinterpret_cast<float*>(p);
                        pf[0] = v ? mesh.c[v - 1].x : 1.F;
                        pf[1] = v ? mesh.c[v - 1].y : 1.F;
                        pf[2] = v ? mesh.c[v - 1].z : 1.F;
                        p += 3 * sizeof(float);
                    }
                }
                break;
            default: break;
            }
            return out;
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

        size_t VBO::getSize() const
        {
            return _size;
        }

        VBOType VBO::getType() const
        {
            return _type;
        }

        GLuint VBO::getID() const
        {
            return _vbo;
        }

        GLuint VAO::getID() const
        {
            return _vao;
        }

        void VBO::copy(const std::vector<uint8_t>& data)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizei>(data.size()), (void*)data.data());
        }

        void VBO::copy(const std::vector<uint8_t>& data, std::size_t offset, std::size_t size)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferSubData(GL_ARRAY_BUFFER, offset, static_cast<GLsizei>(size), (void*)data.data());
        }

        void VAO::_init(VBOType type, GLuint vbo)
        {
            glGenVertexArrays(1, &_vao);
            glBindVertexArray(_vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            const std::size_t byteCount = getByteCount(type);
            switch (type)
            {
            case VBOType::Pos2_F32:
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(byteCount), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                break;
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
