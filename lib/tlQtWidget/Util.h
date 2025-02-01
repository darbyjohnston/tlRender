// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Size.h>

#include <dtk/core/Color.h>

#include <QColor>
#include <QSize>
#include <QWidget>

namespace tl
{
    namespace qtwidget
    {
        //! Convert to a Qt size.
        QSize toQt(const math::Size2i&);

        //! Convert from a Qt size.
        math::Size2i fromQt(const QSize&);

        //! Convert to a Qt color.
        QColor toQt(const dtk::Color4F&);

        //! Convert from a Qt color.
        dtk::Color4F fromQt(const QColor&);

        //! Set whether the window is floating on top.
        void setFloatOnTop(bool, QWidget*);
    }
}
