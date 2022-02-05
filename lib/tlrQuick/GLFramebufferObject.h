// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQuick/Util.h>

#include <tlrCore/Timeline.h>

#include <QQuickFramebufferObject>

namespace tlr
{
    namespace quick
    {
        //! OpenGL frame buffer object.
        class GLFramebufferObject : public QQuickFramebufferObject
        {
            Q_OBJECT
            Q_PROPERTY(
                tlr::timeline::VideoData video
                READ video
                WRITE setVideo)

        public:
            GLFramebufferObject(QQuickItem* parent = nullptr);

            ~GLFramebufferObject() override;
            
            //! Get the video data.
            const tlr::timeline::VideoData& video() const;

            Renderer* createRenderer() const override;

        public Q_SLOTS:
            //! Set the video data.
            void setVideo(const tlr::timeline::VideoData&);

        private:
            TLR_PRIVATE();
        };
    }
}

