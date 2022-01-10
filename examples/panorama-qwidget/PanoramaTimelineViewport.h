// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelinePlayer.h>

#include <tlrGL/Mesh.h>
#include <tlrGL/OffscreenBuffer.h>
#include <tlrGL/Render.h>
#include <tlrGL/Shader.h>

#include <tlrCore/Mesh.h>
#include <tlrCore/OCIO.h>

#include <QOpenGLWidget>

class PanoramaTimelineViewport : public QOpenGLWidget
{
    Q_OBJECT

public:
    PanoramaTimelineViewport(
        const std::shared_ptr<tlr::core::Context>&,
        QWidget* parent = nullptr);

    //! Set the color configuration.
    void setColorConfig(const tlr::imaging::ColorConfig&);

    //! Set the image options.
    void setImageOptions(const tlr::render::ImageOptions&);

    //! Set the timeline player.
    void setTimelinePlayer(tlr::qt::TimelinePlayer*);

private Q_SLOTS:
    void _videoCallback(const tlr::timeline::VideoData&);

protected:
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private:
    std::weak_ptr<tlr::core::Context> _context;
    tlr::imaging::ColorConfig _colorConfig;
    tlr::render::ImageOptions _imageOptions;
    tlr::qt::TimelinePlayer* _timelinePlayer = nullptr;
    tlr::imaging::Size _videoSize;
    tlr::timeline::VideoData _videoData;
    glm::vec2 _cameraRotation = glm::vec2(0.F, 0.F);
    float _cameraFOV = 45.F;
    tlr::geom::TriangleMesh3 _sphereMesh;
    std::shared_ptr<tlr::gl::VBO> _sphereVBO;
    std::shared_ptr<tlr::gl::VAO> _sphereVAO;
    std::shared_ptr<tlr::gl::Shader> _shader;
    std::shared_ptr<tlr::gl::OffscreenBuffer> _buffer;
    std::shared_ptr<tlr::gl::Render> _render;
    QPoint _mousePosPrev;
};
