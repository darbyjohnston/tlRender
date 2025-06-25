// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/PlayerObject.h>

#include <tlTimelineGL/Render.h>

#include <feather-tk/gl/Mesh.h>
#include <feather-tk/gl/OffscreenBuffer.h>
#include <feather-tk/gl/Shader.h>

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
                    const std::shared_ptr<feather_tk::Context>&,
                    QWidget* parent = nullptr);

                //! Set the OpenColorIO options.
                void setOCIOOptions(const timeline::OCIOOptions&);

                //! Set the LUT options.
                void setLUTOptions(const timeline::LUTOptions&);

                //! Set the image options.
                void setImageOptions(const feather_tk::ImageOptions&);

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
                std::weak_ptr<feather_tk::Context> _context;
                timeline::OCIOOptions _ocioOptions;
                timeline::LUTOptions _lutOptions;
                feather_tk::ImageOptions _imageOptions;
                QSharedPointer<qt::PlayerObject> _player;
                feather_tk::Size2I _videoSize;
                std::vector<timeline::VideoData> _videoData;
                feather_tk::V2F _cameraRotation;
                float _cameraFOV = 45.F;
                feather_tk::TriMesh3F _sphereMesh;
                std::shared_ptr<feather_tk::gl::VBO> _sphereVBO;
                std::shared_ptr<feather_tk::gl::VAO> _sphereVAO;
                std::shared_ptr<feather_tk::gl::Shader> _shader;
                std::shared_ptr<feather_tk::gl::OffscreenBuffer> _buffer;
                std::shared_ptr<timeline_gl::Render> _render;
                feather_tk::V2I _mousePosPrev;
            };
        }
    }
}
