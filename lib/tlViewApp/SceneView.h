// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/TimelineItem.h>

#include <tlGlad/gl.h>

#include <QOpenGLWidget>

namespace tl
{
    namespace view
    {
        //! Scene view.
        class SceneView : public QOpenGLWidget
        {
            Q_OBJECT

        public:
            SceneView(
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            ~SceneView() override;

            void setScene(const std::shared_ptr<TimelineItem>&);

        protected:
            void initializeGL() override;
            void resizeGL(int w, int h) override;
            void paintGL() override;

        private:
            imaging::Size _viewportSize() const;

            TLRENDER_PRIVATE();
        };
    }
}
