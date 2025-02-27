// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PanoramaViewport.h"

#include <dtk/gl/Init.h>

#include <QMouseEvent>
#include <QSurfaceFormat>

namespace tl
{
    namespace examples
    {
        namespace panorama_qtwidget
        {
            PanoramaViewport::PanoramaViewport(
                const std::shared_ptr<dtk::Context>& context,
                QWidget* parent) :
                QOpenGLWidget(parent)
            {
                _context = context;

                QSurfaceFormat surfaceFormat;
                surfaceFormat.setMajorVersion(4);
                surfaceFormat.setMinorVersion(1);
                surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
                setFormat(surfaceFormat);
            }

            void PanoramaViewport::setOCIOOptions(const timeline::OCIOOptions& value)
            {
                if (value == _ocioOptions)
                    return;
                _ocioOptions = value;
                update();
            }

            void PanoramaViewport::setLUTOptions(const timeline::LUTOptions& value)
            {
                if (value == _lutOptions)
                    return;
                _lutOptions = value;
                update();
            }

            void PanoramaViewport::setImageOptions(const dtk::ImageOptions& value)
            {
                if (value == _imageOptions)
                    return;
                _imageOptions = value;
                update();
            }

            void PanoramaViewport::setPlayer(const QSharedPointer<qt::PlayerObject>& player)
            {
                if (_player)
                {
                    disconnect(
                        _player.get(),
                        SIGNAL(currentVideoChanged(const std::vector<tl::timeline::VideoData>&)),
                        this,
                        SLOT(_currentVideoCallback(const std::vector<tl::timeline::VideoData>&)));
                }
                _player = player;
                _videoData.clear();
                if (_player)
                {
                    const auto& ioInfo = _player->ioInfo();
                    _videoSize = !ioInfo.video.empty() ? ioInfo.video[0].size : dtk::Size2I();
                    _videoData = _player->currentVideo();
                    connect(
                        _player.get(),
                        SIGNAL(currentVideoChanged(const std::vector<tl::timeline::VideoData>&)),
                        SLOT(_currentVideoCallback(const std::vector<tl::timeline::VideoData>&)));
                }
                update();
            }

            void PanoramaViewport::_currentVideoCallback(const std::vector<timeline::VideoData>& value)
            {
                _videoData = value;
                update();
            }

            void PanoramaViewport::initializeGL()
            {
                initializeOpenGLFunctions();
                dtk::gl::initGLAD();

                try
                {
                    // Create the sphere mesh.
                    _sphereMesh = dtk::sphere(10.F, 100, 100);
                    auto vboData = dtk::gl::convert(
                        _sphereMesh,
                        dtk::gl::VBOType::Pos3_F32_UV_U16,
                        dtk::RangeSizeT(0, _sphereMesh.triangles.size() - 1));
                    _sphereVBO = dtk::gl::VBO::create(_sphereMesh.triangles.size() * 3, dtk::gl::VBOType::Pos3_F32_UV_U16);
                    _sphereVBO->copy(vboData);
                    _sphereVAO = dtk::gl::VAO::create(dtk::gl::VBOType::Pos3_F32_UV_U16, _sphereVBO->getID());

                    // Create the renderer.
                    if (auto context = _context.lock())
                    {
                        _render = timeline_gl::Render::create(context);
                    }

                    // Create the shader.
                    const std::string vertexSource =
                        "#version 410\n"
                        "\n"
                        "// Inputs\n"
                        "in vec3 vPos;\n"
                        "in vec2 vTexture;\n"
                        "\n"
                        "// Outputs\n"
                        "out vec2 fTexture;\n"
                        "\n"
                        "// Uniforms\n"
                        "uniform struct Transform\n"
                        "{\n"
                        "    mat4 mvp;\n"
                        "} transform;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                        "    fTexture = vTexture;\n"
                        "}\n";
                    const std::string fragmentSource =
                        "#version 410\n"
                        "\n"
                        "// Inputs\n"
                        "in vec2 fTexture;\n"
                        "\n"
                        "// Outputs\n"
                        "out vec4 fColor;\n"
                        "\n"
                        "// Uniforms\n"
                        "uniform sampler2D textureSampler;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    fColor = texture(textureSampler, fTexture);\n"
                        "}\n";
                    _shader = dtk::gl::Shader::create(vertexSource, fragmentSource);
                }
                catch (const std::exception& e)
                {
                    // Re-throw the exception to be caught in main().
                    throw e;
                }
            }

            void PanoramaViewport::paintGL()
            {
                try
                {
                    // Create the offscreen buffer.
                    dtk::Size2I offscreenBufferSize(_videoSize.w, _videoSize.h);
                    dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.color = dtk::ImageType::RGBA_F32;
                    if (dtk::gl::doCreate(_buffer, offscreenBufferSize, offscreenBufferOptions))
                    {
                        _buffer = dtk::gl::OffscreenBuffer::create(offscreenBufferSize, offscreenBufferOptions);
                    }

                    // Render the video data into the offscreen buffer.
                    if (_buffer)
                    {
                        dtk::gl::OffscreenBufferBinding binding(_buffer);
                        _render->begin(offscreenBufferSize);
                        _render->setOCIOOptions(_ocioOptions);
                        _render->setLUTOptions(_lutOptions);
                        _render->drawVideo(
                            { _videoData },
                            { dtk::Box2I(0, 0, _videoSize.w, _videoSize.h) },
                            { _imageOptions });
                        _render->end();
                    }
                }
                catch (const std::exception& e)
                {
                    // Re-throw the exception to be caught in main().
                    throw e;
                }

                // Render a sphere using the offscreen buffer as a texture.
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_BLEND);
                const float devicePixelRatio = window()->devicePixelRatio();
                const dtk::Size2I windowSize(
                    width() * devicePixelRatio,
                    height() * devicePixelRatio);
                glViewport(
                    0,
                    0,
                    static_cast<GLsizei>(windowSize.w),
                    static_cast<GLsizei>(windowSize.h));
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);
                dtk::M44F vm;
                vm = vm * dtk::translate(dtk::V3F(0.F, 0.F, 0.F));
                vm = vm * dtk::rotateX(_cameraRotation.x);
                vm = vm * dtk::rotateY(_cameraRotation.y);
                const auto pm = dtk::perspective(
                    _cameraFOV,
                    windowSize.w / static_cast<float>(windowSize.h > 0 ? windowSize.h : 1),
                    .1F,
                    10000.F);
                _shader->bind();
                _shader->setUniform("transform.mvp", pm * vm);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, _buffer->getColorID());
                _sphereVAO->bind();
                _sphereVAO->draw(GL_TRIANGLES, 0, _sphereMesh.triangles.size() * 3);
            }

            void PanoramaViewport::mousePressEvent(QMouseEvent* event)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                _mousePosPrev.x = event->x() * devicePixelRatio;
                _mousePosPrev.y = event->y() * devicePixelRatio;
            }

            void PanoramaViewport::mouseReleaseEvent(QMouseEvent*)
            {}

            void PanoramaViewport::mouseMoveEvent(QMouseEvent* event)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                _cameraRotation.x += (event->y() * devicePixelRatio - _mousePosPrev.y) / 20.F * -1.F;
                _cameraRotation.y += (event->x() * devicePixelRatio - _mousePosPrev.x) / 20.F * -1.F;
                _mousePosPrev.x = event->x() * devicePixelRatio;
                _mousePosPrev.y = event->y() * devicePixelRatio;
            }
        }
    }
}
