// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Color.h>
#include <dtk/core/Util.h>

#include <QDialog>

namespace tl
{
    namespace qtwidget
    {
        //! Color picker dialog.
        class ColorDialog : public QDialog
        {
            Q_OBJECT

        public:
            ColorDialog(
                const dtk::Color4F& = dtk::Color4F(),
                QWidget* parent = nullptr);

            virtual ~ColorDialog();

            //! Get the color.
            const dtk::Color4F& color() const;

        private:
            DTK_PRIVATE();
        };
    }
}
