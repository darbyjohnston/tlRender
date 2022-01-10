// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelinePlayer.h>

#include <tlrCore/IRender.h>
#include <tlrCore/OCIO.h>

#include <QOpenGLWidget>

namespace tlr
{
    namespace qwidget
    {
        //! Timeline viewport widget.
        class TimelineViewport : public QOpenGLWidget
        {
            Q_OBJECT

        public:
            TimelineViewport(
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);
            
            ~TimelineViewport() override;

            //! Set the color configuration.
            void setColorConfig(const imaging::ColorConfig&);

            //! Set the image options.
            void setImageOptions(const render::ImageOptions&);

            //! Set the timeline player.
            void setTimelinePlayer(qt::TimelinePlayer*);

        private Q_SLOTS:
            void _videoCallback(const tlr::timeline::VideoData&);

        protected:
            void initializeGL() override;
            void paintGL() override;

        private:
            TLR_PRIVATE();
        };
    }
}
