// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineViewport.h>

#include <tlrCore/BBox.h>
#include <tlrCore/Matrix.h>

#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLVertexArrayObject>

namespace tlr
{
    namespace qt
    {
        TimelineViewport::TimelineViewport(QWidget* parent) :
            QOpenGLWidget(parent)
        {}

        void TimelineViewport::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            _frame = io::VideoFrame();
            _frameTmp = io::VideoFrame();
            if (_timelinePlayer)
            {
                disconnect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::io::VideoFrame&)));
            }
            _timelinePlayer = timelinePlayer;
            if (_timelinePlayer)
            {
                connect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::io::VideoFrame&)),
                    SLOT(_frameCallback(const tlr::io::VideoFrame&)));
            }
            update();
        }

        void TimelineViewport::_frameCallback(const io::VideoFrame& frame)
        {
            _frameTmp = frame;
            update();
        }

        void TimelineViewport::initializeGL()
        {
            initializeOpenGLFunctions();

            QOpenGLShader vertexShader(QOpenGLShader::Vertex);
            vertexShader.compileSourceCode(
                "attribute vec3 aPos;\n"
                "attribute vec2 aTexture;\n"
                "\n"
                "varying vec2 texture;\n"
                "\n"
                "uniform mat4 mvp;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = mvp * vec4(aPos, 1.0);\n"
                "    texture = aTexture;\n"
                "}\n");
            QOpenGLShader fragmentShader(QOpenGLShader::Fragment);
            fragmentShader.compileSourceCode(
                "varying vec2 texture;\n"
                "\n"
                "// tlr::imaging::PixelType\n"
                "#define PIXEL_TYPE_NONE     0\n"
                "#define PIXEL_TYPE_L_U8     1\n"
                "#define PIXEL_TYPE_L_U16    2\n"
                "#define PIXEL_TYPE_L_F32    3\n"
                "#define PIXEL_TYPE_LA_U8    4\n"
                "#define PIXEL_TYPE_LA_U16   5\n"
                "#define PIXEL_TYPE_LA_F32   6\n"
                "#define PIXEL_TYPE_RGB_U8   7\n"
                "#define PIXEL_TYPE_RGB_U16  8\n"
                "#define PIXEL_TYPE_RGB_F32  9\n"
                "#define PIXEL_TYPE_RGBA_U8  10\n"
                "#define PIXEL_TYPE_RGBA_U16 11\n"
                "#define PIXEL_TYPE_RGBA_F16 12\n"
                "#define PIXEL_TYPE_RGBA_F32 13\n"
                "#define PIXEL_TYPE_YUV_420P 14\n"
                "uniform int pixelType;\n"
                "uniform sampler2D textureSampler0;\n"
                "uniform sampler2D textureSampler1;\n"
                "uniform sampler2D textureSampler2;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    if (PIXEL_TYPE_YUV_420P == pixelType)\n"
                "    {\n"
                "        float y = texture2D(textureSampler0, texture).r;\n"
                "        float u = texture2D(textureSampler1, texture).r - 0.5;\n"
                "        float v = texture2D(textureSampler2, texture).r - 0.5;\n"
                "        gl_FragColor.r = y + 1.402 * v;\n"
                "        gl_FragColor.g = y - 0.344 * u - 0.714 * v;\n"
                "        gl_FragColor.b = y + 1.772 * u;\n"
                "        gl_FragColor.a = 1.0;\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        vec4 t = texture2D(textureSampler0, texture);\n"
                "        gl_FragColor = t;\n"
                "    }\n"
                "}\n");

            _program = new QOpenGLShaderProgram;
            _program->addShader(&vertexShader);
            _program->addShader(&fragmentShader);
            _program->bindAttributeLocation("aPos", 0);
            _program->bindAttributeLocation("aTexture", 1);
            _program->link();

            for (auto& i : _textures)
            {
                i.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
                i->create();
            }
        }

        namespace
        {
            struct VBOVertex
            {
                float    vx;
                float    vy;
                uint16_t tx;
                uint16_t ty;
            };

            QOpenGLTexture::TextureFormat getTextureFormat(imaging::PixelType value)
            {
                const std::array<QOpenGLTexture::TextureFormat, 15> values =
                {
                    QOpenGLTexture::NoFormat,
                    QOpenGLTexture::R8_UNorm,
                    QOpenGLTexture::R16_UNorm,
                    QOpenGLTexture::R32F,
                    QOpenGLTexture::RG8_UNorm,
                    QOpenGLTexture::RG16_UNorm,
                    QOpenGLTexture::RG32F,
                    QOpenGLTexture::RGB8_UNorm,
                    QOpenGLTexture::RGB16_UNorm,
                    QOpenGLTexture::RGB32F,
                    QOpenGLTexture::RGBA8_UNorm,
                    QOpenGLTexture::RGBA16_UNorm,
                    QOpenGLTexture::RGBA16F,
                    QOpenGLTexture::RGBA32F,
                    QOpenGLTexture::NoFormat
                };
                return values[static_cast<size_t>(value)];
            }

            QOpenGLTexture::PixelFormat getPixelFormat(imaging::PixelType value)
            {
                const std::array<QOpenGLTexture::PixelFormat, 15> values =
                {
                    QOpenGLTexture::NoSourceFormat,
                    QOpenGLTexture::Red,
                    QOpenGLTexture::Red,
                    QOpenGLTexture::Red,
                    QOpenGLTexture::RG,
                    QOpenGLTexture::RG,
                    QOpenGLTexture::RG,
                    QOpenGLTexture::RGB,
                    QOpenGLTexture::RGB,
                    QOpenGLTexture::RGB,
                    QOpenGLTexture::RGBA,
                    QOpenGLTexture::RGBA,
                    QOpenGLTexture::RGBA,
                    QOpenGLTexture::RGBA,
                    QOpenGLTexture::NoSourceFormat
                };
                return values[static_cast<size_t>(value)];
            }

            QOpenGLTexture::PixelType getPixelType(imaging::PixelType value)
            {
                const std::array<QOpenGLTexture::PixelType, 15> values =
                {
                    QOpenGLTexture::NoPixelType,
                    QOpenGLTexture::UInt8,
                    QOpenGLTexture::UInt16,
                    QOpenGLTexture::Float32,
                    QOpenGLTexture::UInt8,
                    QOpenGLTexture::UInt16,
                    QOpenGLTexture::Float32,
                    QOpenGLTexture::UInt8,
                    QOpenGLTexture::UInt16,
                    QOpenGLTexture::Float32,
                    QOpenGLTexture::UInt8,
                    QOpenGLTexture::UInt16,
                    QOpenGLTexture::Float16,
                    QOpenGLTexture::Float32,
                    QOpenGLTexture::NoPixelType
                };
                return values[static_cast<size_t>(value)];
            }
        }

        void TimelineViewport::paintGL()
        {
            const int w = width();
            const int h = height();
            glViewport(0, 0, w, h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            if (_frameTmp != _frame)
            {
                imaging::Info info;
                if (_frame.image)
                {
                    info = _frame.image->getInfo();
                }
                _frame = _frameTmp;
                if (_frame.image)
                {
                    //std::cout << "image: " << _frame.time << std::endl;
                    if (info != _frame.image->getInfo())
                    {
                        info = _frame.image->getInfo();
                        for (auto& i : _textures)
                        {
                            i->destroy();
                        }
                        switch (info.pixelType)
                        {
                            case imaging::PixelType::YUV_420P:
                            {
                                _textures[0]->setSize(info.size.w, info.size.h);
                                _textures[1]->setSize(info.size.w / 2, info.size.h / 2);
                                _textures[2]->setSize(info.size.w / 2, info.size.h / 2);
                                for (auto& i : _textures)
                                {
                                    i->setFormat(QOpenGLTexture::R8_UNorm);
                                    i->allocateStorage();
                                }
                                break;
                            }
                            default:
                                _textures[0]->setSize(info.size.w, info.size.h);
                                _textures[0]->setFormat(getTextureFormat(info.pixelType));
                                _textures[0]->allocateStorage();
                                break;
                        }
                    }
                    switch (info.pixelType)
                    {
                    case imaging::PixelType::YUV_420P:
                    {
                        const uint8_t* data = _frame.image->getData();
                        _textures[0]->bind();
                        _textures[0]->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, data);
                        data += static_cast<std::size_t>(info.size.w) * static_cast<std::size_t>(info.size.h);
                        _textures[1]->bind();
                        _textures[1]->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, data);
                        data += static_cast<std::size_t>(info.size.w / 2) * static_cast<std::size_t>(info.size.h / 2);
                        _textures[2]->bind();
                        _textures[2]->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, data);
                        break;
                    }
                    default:
                        _textures[0]->bind();
                        _textures[0]->setData(getPixelFormat(info.pixelType), getPixelType(info.pixelType), _frame.image->getData());
                        break;
                    }
                }
            }

            if (_frame.image)
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                const imaging::Info& info = _frame.image->getInfo();
                switch (info.pixelType)
                {
                case imaging::PixelType::YUV_420P:
                    _textures[0]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                    _textures[1]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE2));
                    _textures[2]->bind();
                    break;
                default:
                    _textures[0]->bind();
                    break;
                }

                _program->bind();
                QMatrix4x4 m;
                m.ortho(0.F, static_cast<float>(w), static_cast<float>(h), 0.F, -1.F, 1.F);
                _program->setUniformValue("mvp", m);
                _program->setUniformValue("pixelType", static_cast<int>(info.pixelType));
                _program->setUniformValue("textureSampler0", 0);
                _program->setUniformValue("textureSampler1", 1);
                _program->setUniformValue("textureSampler2", 2);

                const math::BBox2f bbox = timeline::fitWindow(info.size, imaging::Size(w, h));
                VBOVertex vboData[4];
                vboData[0].vx = bbox.min.x;
                vboData[0].vy = bbox.min.y;
                vboData[0].tx = 0;
                vboData[0].ty = 0;
                vboData[1].vx = bbox.max.x;
                vboData[1].vy = bbox.min.y;
                vboData[1].tx = 65535;
                vboData[1].ty = 0;
                vboData[2].vx = bbox.min.x;
                vboData[2].vy = bbox.max.y;
                vboData[2].tx = 0;
                vboData[2].ty = 65535;
                vboData[3].vx = bbox.max.x;
                vboData[3].vy = bbox.max.y;
                vboData[3].tx = 65535;
                vboData[3].ty = 65535;
                QOpenGLBuffer vbo;
                vbo.create();
                vbo.bind();
                vbo.allocate(vboData, 4 * sizeof(VBOVertex));

                QOpenGLVertexArrayObject vao;
                vao.create();
                vao.bind();
                _program->enableAttributeArray(0);
                _program->enableAttributeArray(1);
                _program->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(VBOVertex));
                _program->setAttributeBuffer(1, GL_UNSIGNED_SHORT, 8, 2, sizeof(VBOVertex));

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
    }
}
