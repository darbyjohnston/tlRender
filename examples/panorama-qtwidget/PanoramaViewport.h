// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/PlayerObject.h>

#include <tlTimelineGL/Render.h>

#include <dtk/gl/Mesh.h>
#include <dtk/gl/OffscreenBuffer.h>
#include <dtk/gl/Shader.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace examples
    {
        //! Example panorama timeline rendering.
        namespace panorama_qtwidget
        {
            //! Panorama timeline viewport.
            class PanoramaViewport :
                public QOpenGLWidget,
                protected QOpenGLFunctions_4_1_Core
            {
                Q_OBJECT

            public:
                PanoramaViewport(
                    const std::shared_ptr<dtk::Context>&,
                    QWidget* parent = nullptr);

                //! Set the OpenColorIO options.
                void setOCIOOptions(const timeline::OCIOOptions&);

                //! Set the LUT options.
                void setLUTOptions(const timeline::LUTOptions&);

                //! Set the image options.
                void setImageOptions(const dtk::ImageOptions&);

                //! Set the timeline player.
                void setPlayer(const QSharedPointer<qt::PlayerObject>&);

            private Q_SLOTS:
                void _currentVideoCallback(const std::vector<tl::timeline::VideoData>&);

            protected:
                void initializeGL() override;
                void paintGL() override;
                void mousePressEvent(QMouseEvent*) override;
                void mouseReleaseEvent(QMouseEvent*) override;
                void mouseMoveEvent(QMouseEvent*) override;

            private:
                std::weak_ptr<dtk::Context> _context;
                timeline::OCIOOptions _ocioOptions;
                timeline::LUTOptions _lutOptions;
                dtk::ImageOptions _imageOptions;
                QSharedPointer<qt::PlayerObject> _player;
                dtk::Size2I _videoSize;
                std::vector<timeline::VideoData> _videoData;
                dtk::V2F _cameraRotation;
                float _cameraFOV = 45.F;
                dtk::TriMesh3F _sphereMesh;
                std::shared_ptr<dtk::gl::VBO> _sphereVBO;
                std::shared_ptr<dtk::gl::VAO> _sphereVAO;
                std::shared_ptr<dtk::gl::Shader> _shader;
                std::shared_ptr<dtk::gl::OffscreenBuffer> _buffer;
                std::shared_ptr<timeline_gl::Render> _render;
                dtk::V2I _mousePosPrev;
            };
        }
    }
}
