// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Color.h>
#include <dtk/core/Util.h>

#include <QWidget> 

namespace tl
{
    namespace qtwidget
    {
        //! Color swatch.
        class ColorSwatch : public QWidget
        {
            Q_OBJECT

        public:
            ColorSwatch(QWidget* parent = nullptr);

            virtual ~ColorSwatch();

            //! Get the color.
            const dtk::Color4F& color() const;

            //! Set the size of the swatch.
            void setSwatchSize(int);

            //! Set whether the color is editable.
            void setEditable(bool);

            QSize minimumSizeHint() const override;

        public Q_SLOTS:
            //! Set the color.
            void setColor(const dtk::Color4F&);

        Q_SIGNALS:
            //! This signal is emitted when the color is changed.
            void colorChanged(const dtk::Color4F&);

        protected:
            void paintEvent(QPaintEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;

        private:
            DTK_PRIVATE();
        };
    }
}
