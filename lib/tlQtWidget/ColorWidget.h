// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <dtk/core/Color.h>

#include <QWidget> 

namespace tl
{
    namespace qtwidget
    {
        //! Color widget.
        class ColorWidget : public QWidget
        {
            Q_OBJECT

        public:
            ColorWidget(QWidget* parent = nullptr);

            virtual ~ColorWidget();

            //! Get the color.
            const dtk::Color4F& color() const;

        public Q_SLOTS:
            //! Set the color.
            void setColor(const dtk::Color4F&);

        Q_SIGNALS:
            //! This signal is emitted when the color is changed.
            void colorChanged(const dtk::Color4F&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
