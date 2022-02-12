// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlCore/IRender.h>
#include <tlCore/OCIO.h>

#include <QOpenGLWidget>

namespace tl
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
            void setImageOptions(const std::vector<render::ImageOptions>&);

            //! Set the comparison options.
            void setCompareOptions(const render::CompareOptions&);

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

        private Q_SLOTS:
            void _videoCallback(const tl::timeline::VideoData&);

        protected:
            void initializeGL() override;
            void paintGL() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
