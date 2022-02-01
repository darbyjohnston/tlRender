// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "PanoramaTimelineViewport.h"

#include <tlrGL/Mesh.h>

#include <QMouseEvent>
#include <QSurfaceFormat>

#include <glm/gtc/matrix_transform.hpp>

using namespace tlr;

PanoramaTimelineViewport::PanoramaTimelineViewport(
    const std::shared_ptr<core::Context>& context,
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

void PanoramaTimelineViewport::setColorConfig(const imaging::ColorConfig& colorConfig)
{
    if (colorConfig == _colorConfig)
        return;
    _colorConfig = colorConfig;
    update();
}

void PanoramaTimelineViewport::setImageOptions(const render::ImageOptions& imageOptions)
{
    if (imageOptions == _imageOptions)
        return;
    _imageOptions = imageOptions;
    update();
}

void PanoramaTimelineViewport::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
{
    _videoData = timeline::VideoData();
    if (_timelinePlayer)
    {
        disconnect(
            _timelinePlayer,
            SIGNAL(videoChanged(const tlr::timeline::VideoData&)),
            this,
            SLOT(_videoCallback(const tlr::timeline::VideoData&)));
    }
    _timelinePlayer = timelinePlayer;
    if (_timelinePlayer)
    {
        const auto& avInfo = _timelinePlayer->avInfo();
        _videoSize = !avInfo.video.empty() ? avInfo.video[0].size : imaging::Size();
        _videoData = _timelinePlayer->video();
        connect(
            _timelinePlayer,
            SIGNAL(videoChanged(const tlr::timeline::VideoData&)),
            SLOT(_videoCallback(const tlr::timeline::VideoData&)));
    }
    update();
}

void PanoramaTimelineViewport::_videoCallback(const timeline::VideoData& value)
{
    _videoData = value;
    update();
}

void PanoramaTimelineViewport::initializeGL()
{
    // Initialize GLAD.
    gladLoaderLoadGL();

    try
    {
        // Create the sphere mesh.
        _sphereMesh = geom::createSphere(10.F, 100, 100);
        auto vboData = convert(
            _sphereMesh,
            gl::VBOType::Pos3_F32_UV_U16,
            math::SizeTRange(0, _sphereMesh.triangles.size() - 1));
        _sphereVBO = gl::VBO::create(_sphereMesh.triangles.size() * 3, gl::VBOType::Pos3_F32_UV_U16);
        _sphereVBO->copy(vboData);
        _sphereVAO = gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, _sphereVBO->getID());

        // Create the renderer.
        if (auto context = _context.lock())
        {
            _render = gl::Render::create(context);
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
        _shader = gl::Shader::create(vertexSource, fragmentSource);
    }
    catch (const std::exception& e)
    {
        // Re-throw the exception to be caught in main().
        throw e;
    }
}

void PanoramaTimelineViewport::paintGL()
{
    try
    {
        // Create the offscreen buffer.
        if (!_buffer || (_buffer && _buffer->getSize() != _videoSize))
        {
            _buffer = gl::OffscreenBuffer::create(_videoSize, imaging::PixelType::RGBA_F32);
        }

        // Render the video data into the offscreen buffer.
        {
            gl::OffscreenBufferBinding binding(_buffer);
            _render->setColorConfig(_colorConfig);
            _render->begin(_videoSize);
            _render->drawVideo({ _videoData }, { _imageOptions });
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
    const QSize& windowSize = size();
    glViewport(
        0,
        0,
        GLsizei(windowSize.width()),
        GLsizei(windowSize.height()));
    glClearColor(1.F, 0.F, 0.F, 0.F);
    glClear(GL_COLOR_BUFFER_BIT);
    glm::mat4x4 v(1.F);
    glm::mat4x4 p(1.F);
    v = glm::translate(v, glm::vec3(0.F, 0.F, 0.F));
    v = glm::rotate(v, math::deg2rad(_cameraRotation.x), glm::vec3(1.F, 0.F, 0.F));
    v = glm::rotate(v, math::deg2rad(_cameraRotation.y), glm::vec3(0.F, 1.F, 0.F));
    p = glm::perspective(
        math::deg2rad(_cameraFOV),
        windowSize.width() / static_cast<float>(windowSize.height() > 0 ? windowSize.height() : 1),
        .1F,
        10000.F);
    _shader->bind();
    const glm::mat4x4 vp = p * v;
    _shader->setUniform(
        "transform.mvp",
        math::Matrix4x4f(
            vp[0][0], vp[0][1], vp[0][2], vp[0][3],
            vp[1][0], vp[1][1], vp[1][2], vp[1][3],
            vp[2][0], vp[2][1], vp[2][2], vp[2][3],
            vp[3][0], vp[3][1], vp[3][2], vp[3][3]));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _buffer->getColorID());
    _sphereVAO->bind();
    _sphereVAO->draw(GL_TRIANGLES, 0, _sphereMesh.triangles.size() * 3);
}

void PanoramaTimelineViewport::mousePressEvent(QMouseEvent* event)
{
    _mousePosPrev = event->pos();
}

void PanoramaTimelineViewport::mouseReleaseEvent(QMouseEvent*)
{}

void PanoramaTimelineViewport::mouseMoveEvent(QMouseEvent* event)
{
    _cameraRotation.x += (event->pos().y() - _mousePosPrev.y()) / 10.F * -1.F;
    _cameraRotation.y += (event->pos().x()  - _mousePosPrev.x()) / 10.F * -1.F;
    _mousePosPrev = event->pos();
}
