// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtQuick/Util.h>

#include <tlTimeline/Timeline.h>

#include <QQuickFramebufferObject>

namespace tl
{
    namespace qt
    {
        namespace quick
        {
            //! OpenGL frame buffer object.
            class GLFramebufferObject : public QQuickFramebufferObject
            {
                Q_OBJECT
                    Q_PROPERTY(
                        tl::timeline::VideoData video
                        READ video
                        WRITE setVideo)

            public:
                GLFramebufferObject(QQuickItem* parent = nullptr);

                ~GLFramebufferObject() override;

                //! Get the video data.
                const timeline::VideoData& video() const;

                Renderer* createRenderer() const override;

            public Q_SLOTS:
                //! Set the video data.
                void setVideo(const tl::timeline::VideoData&);

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
