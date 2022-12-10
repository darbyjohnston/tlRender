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

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace examples
    {
        //! Example rendering a timeline as a panorama.
        namespace panorama_qtwidget
        {
            class PanoramaTimelineViewport :
                public QOpenGLWidget,
                protected QOpenGLFunctions_4_1_Core
            {
                Q_OBJECT

            public:
                PanoramaTimelineViewport(
                    const std::shared_ptr<system::Context>&,
                    QWidget* parent = nullptr);

                //! Set the color configuration options.
                void setColorConfigOptions(const timeline::ColorConfigOptions&);

                //! Set the LUT options.
                void setLUTOptions(const timeline::LUTOptions&);

                //! Set the image options.
                void setImageOptions(const timeline::ImageOptions&);

                //! Set the timeline player.
                void setTimelinePlayer(qt::TimelinePlayer*);

            private Q_SLOTS:
                void _currentVideoCallback(const tl::timeline::VideoData&);

            protected:
                void initializeGL() override;
                void paintGL() override;
                void mousePressEvent(QMouseEvent*) override;
                void mouseReleaseEvent(QMouseEvent*) override;
                void mouseMoveEvent(QMouseEvent*) override;

            private:
                std::weak_ptr<system::Context> _context;
                timeline::ColorConfigOptions _colorConfigOptions;
                timeline::LUTOptions _lutOptions;
                timeline::ImageOptions _imageOptions;
                qt::TimelinePlayer* _timelinePlayer = nullptr;
                imaging::Size _videoSize;
                timeline::VideoData _videoData;
                math::Vector2f _cameraRotation;
                float _cameraFOV = 45.F;
                geom::TriangleMesh3 _sphereMesh;
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
