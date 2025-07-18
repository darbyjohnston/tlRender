// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PanoramaViewport.h"

#include <feather-tk/gl/Init.h>

#include <QMouseEvent>
#include <QSurfaceFormat>

namespace tl
{
    namespace examples
    {
        namespace panorama_qtwidget
        {
            PanoramaViewport::PanoramaViewport(
                const std::shared_ptr<feather_tk::Context>& context,
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

            void PanoramaViewport::setImageOptions(const feather_tk::ImageOptions& value)
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
                    _videoSize = !ioInfo.video.empty() ? ioInfo.video[0].size : feather_tk::Size2I();
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
                feather_tk::gl::initGLAD();

                // Create the sphere mesh.
                _sphereMesh = feather_tk::sphere(10.F, 100, 100);
                auto vboData = feather_tk::gl::convert(
                    _sphereMesh,
                    feather_tk::gl::VBOType::Pos3_F32_UV_U16,
                    feather_tk::RangeSizeT(0, _sphereMesh.triangles.size() - 1));
                _sphereVBO = feather_tk::gl::VBO::create(_sphereMesh.triangles.size() * 3, feather_tk::gl::VBOType::Pos3_F32_UV_U16);
                _sphereVBO->copy(vboData);
                _sphereVAO = feather_tk::gl::VAO::create(feather_tk::gl::VBOType::Pos3_F32_UV_U16, _sphereVBO->getID());

                // Create the renderer.
                if (auto context = _context.lock())
                {
                    _render = timeline_gl::Render::create(context);
                }

                // Create the shader.
                const std::string vertexSource =
                    "#version 410\n"
                    "\n"
                    "in vec3 vPos;\n"
                    "in vec2 vTexture;\n"
                    "out vec2 fTexture;\n"
                    "\n"
                    "uniform struct Transform\n"
                    "{\n"
                    "    mat4 mvp;\n"
                    "} transform;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    gl_Position = vec4(vPos, 1.0) * transform.mvp;\n"
                    "    fTexture = vTexture;\n"
                    "}\n";
                const std::string fragmentSource =
                    "#version 410\n"
                    "\n"
                    "in vec2 fTexture;\n"
                    "out vec4 fColor;\n"
                    "\n"
                    "uniform sampler2D textureSampler;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    fColor = texture(textureSampler, fTexture);\n"
                    "}\n";
                _shader = feather_tk::gl::Shader::create(vertexSource, fragmentSource);
            }

            void PanoramaViewport::paintGL()
            {
                // Create the offscreen buffer.
                feather_tk::Size2I offscreenBufferSize(_videoSize.w, _videoSize.h);
                feather_tk::gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.color = feather_tk::ImageType::RGBA_F32;
                if (feather_tk::gl::doCreate(_buffer, offscreenBufferSize, offscreenBufferOptions))
                {
                    _buffer = feather_tk::gl::OffscreenBuffer::create(offscreenBufferSize, offscreenBufferOptions);
                }

                // Render the video data into the offscreen buffer.
                if (_buffer)
                {
                    feather_tk::gl::OffscreenBufferBinding binding(_buffer);
                    _render->begin(offscreenBufferSize);
                    _render->setOCIOOptions(_ocioOptions);
                    _render->setLUTOptions(_lutOptions);
                    _render->drawVideo(
                        { _videoData },
                        { feather_tk::Box2I(0, 0, _videoSize.w, _videoSize.h) },
                        { _imageOptions });
                    _render->end();
                }

                // Render a sphere using the offscreen buffer as a texture.
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_BLEND);
                const float devicePixelRatio = window()->devicePixelRatio();
                const feather_tk::Size2I windowSize(
                    width() * devicePixelRatio,
                    height() * devicePixelRatio);
                glViewport(
                    0,
                    0,
                    static_cast<GLsizei>(windowSize.w),
                    static_cast<GLsizei>(windowSize.h));
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);
                feather_tk::M44F vm;
                vm = vm * feather_tk::translate(feather_tk::V3F(0.F, 0.F, 0.F));
                vm = vm * feather_tk::rotateX(_cameraRotation.x);
                vm = vm * feather_tk::rotateY(_cameraRotation.y);
                const auto pm = feather_tk::perspective(
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
