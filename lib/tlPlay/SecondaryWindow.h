// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQWidget/TimelineViewport.h>

#include <QWidget>

namespace tl
{
    namespace play
    {
        //! Secondary window.
        class SecondaryWindow : public QWidget
        {
            Q_OBJECT

        public:
            SecondaryWindow(
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);

            ~SecondaryWindow() override;

            //! Set the color configuration.
            void setColorConfig(const imaging::ColorConfig&);

            //! Set the image options.
            void setImageOptions(const std::vector<render::ImageOptions>&);

            //! Set the comparison options.
            void setCompareOptions(const render::CompareOptions&);

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

        protected:
            void keyPressEvent(QKeyEvent*) override;

        private:
            qwidget::TimelineViewport* _viewport = nullptr;
        };
    }
}
