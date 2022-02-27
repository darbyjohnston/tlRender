// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlRenderGL/Mesh.h>
#include <tlRenderGL/OffscreenBuffer.h>
#include <tlRenderGL/Render.h>
#include <tlRenderGL/Shader.h>

#include <tlCore/Mesh.h>
#include <tlCore/OCIO.h>

#include <QOpenGLWidget>

using namespace tl::core;

namespace tl
{
    namespace examples
    {
        namespace panorama_qtwidget
        {
            class PanoramaTimelineViewport : public QOpenGLWidget
            {
                Q_OBJECT

            public:
                PanoramaTimelineViewport(
                    const std::shared_ptr<Context>&,
                    QWidget* parent = nullptr);

                //! Set the color configuration.
                void setColorConfig(const core::imaging::ColorConfig&);

                //! Set the image options.
                void setImageOptions(const timeline::ImageOptions&);

                //! Set the timeline player.
                void setTimelinePlayer(qt::TimelinePlayer*);

            private Q_SLOTS:
                void _videoCallback(const timeline::VideoData&);

            protected:
                void initializeGL() override;
                void paintGL() override;
                void mousePressEvent(QMouseEvent*) override;
                void mouseReleaseEvent(QMouseEvent*) override;
                void mouseMoveEvent(QMouseEvent*) override;

            private:
                std::weak_ptr<Context> _context;
                core::imaging::ColorConfig _colorConfig;
                timeline::ImageOptions _imageOptions;
                qt::TimelinePlayer* _timelinePlayer = nullptr;
                core::imaging::Size _videoSize;
                timeline::VideoData _videoData;
                core::math::Vector2f _cameraRotation;
                float _cameraFOV = 45.F;
                core::geom::TriangleMesh3 _sphereMesh;
                std::shared_ptr<gl::VBO> _sphereVBO;
                std::shared_ptr<gl::VAO> _sphereVAO;
                std::shared_ptr<gl::Shader> _shader;
                std::shared_ptr<gl::OffscreenBuffer> _buffer;
                std::shared_ptr<gl::Render> _render;
                QPoint _mousePosPrev;
            };
        }
    }
}
