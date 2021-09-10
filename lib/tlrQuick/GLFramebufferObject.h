// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQuick/Util.h>

#include <tlrCore/Timeline.h>

#include <QQuickFramebufferObject>

namespace tlr
{
    namespace quick
    {
        class GLFramebufferObject : public QQuickFramebufferObject
        {
            Q_OBJECT
            Q_PROPERTY(tlr::timeline::Frame frame READ frame WRITE setFrame)

        public:
            GLFramebufferObject(QQuickItem* parent = nullptr);

            ~GLFramebufferObject() override;
            
            const tlr::timeline::Frame& frame() const;

            Renderer* createRenderer() const override;

        public Q_SLOTS:
            void setFrame(const tlr::timeline::Frame&);

        private:
            TLR_PRIVATE();
        };
    }
}

