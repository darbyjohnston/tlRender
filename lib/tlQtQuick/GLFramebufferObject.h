// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <QQuickFramebufferObject>

namespace tl
{
    namespace qtquick
    {
        //! OpenGL frame buffer object.
        class GLFramebufferObject : public QQuickFramebufferObject
        {
            Q_OBJECT
            Q_PROPERTY(
                std::vector<tl::timeline::VideoData> video
                READ video
                WRITE setVideo)

        public:
            GLFramebufferObject(QQuickItem* parent = nullptr);

            virtual ~GLFramebufferObject();

            //! Get the video data.
            const std::vector<timeline::VideoData>& video() const;

            Renderer* createRenderer() const override;

        public Q_SLOTS:
            //! Set the video data.
            void setVideo(const std::vector<tl::timeline::VideoData>&);

        private:
            FTK_PRIVATE();
        };
    }
}
