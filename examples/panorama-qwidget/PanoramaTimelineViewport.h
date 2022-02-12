// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>

#include <tlCore/Mesh.h>
#include <tlCore/OCIO.h>

#include <QOpenGLWidget>

class PanoramaTimelineViewport : public QOpenGLWidget
{
    Q_OBJECT

public:
    PanoramaTimelineViewport(
        const std::shared_ptr<tl::core::Context>&,
        QWidget* parent = nullptr);

    //! Set the color configuration.
    void setColorConfig(const tl::imaging::ColorConfig&);

    //! Set the image options.
    void setImageOptions(const tl::render::ImageOptions&);

    //! Set the timeline player.
    void setTimelinePlayer(tl::qt::TimelinePlayer*);

private Q_SLOTS:
    void _videoCallback(const tl::timeline::VideoData&);

protected:
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private:
    std::weak_ptr<tl::core::Context> _context;
    tl::imaging::ColorConfig _colorConfig;
    tl::render::ImageOptions _imageOptions;
    tl::qt::TimelinePlayer* _timelinePlayer = nullptr;
    tl::imaging::Size _videoSize;
    tl::timeline::VideoData _videoData;
    tl::math::Vector2f _cameraRotation;
    float _cameraFOV = 45.F;
    tl::geom::TriangleMesh3 _sphereMesh;
    std::shared_ptr<tl::gl::VBO> _sphereVBO;
    std::shared_ptr<tl::gl::VAO> _sphereVAO;
    std::shared_ptr<tl::gl::Shader> _shader;
    std::shared_ptr<tl::gl::OffscreenBuffer> _buffer;
    std::shared_ptr<tl::gl::Render> _render;
    QPoint _mousePosPrev;
};
