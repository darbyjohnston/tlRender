// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <tlCore/IRender.h>
#include <tlCore/OCIO.h>

#include <QWidget>

namespace tl
{
    namespace qwidget
    {
        class TimelineViewport;

        //! Timeline widget.
        class TimelineWidget : public QWidget
        {
            Q_OBJECT

        public:
            TimelineWidget(
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);
            
            ~TimelineWidget() override;

            //! Get the viewport.
            TimelineViewport* viewport() const;

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Set the color configuration.
            void setColorConfig(const imaging::ColorConfig&);

            //! Set the image options.
            void setImageOptions(const std::vector<render::ImageOptions>&);

            //! Set the comparison options.
            void setCompareOptions(const render::CompareOptions&);

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

        public Q_SLOTS:
            //! Focus the current frame widget.
            void focusCurrentFrame();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
