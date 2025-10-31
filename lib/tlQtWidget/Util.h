// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/Color.h>
#include <ftk/Core/Size.h>

#include <QColor>
#include <QSize>
#include <QWidget>

namespace tl
{
    namespace qtwidget
    {
        //! Convert to a Qt size.
        QSize toQt(const ftk::Size2I&);

        //! Convert from a Qt size.
        ftk::Size2I fromQt(const QSize&);

        //! Convert to a Qt color.
        QColor toQt(const ftk::Color4F&);

        //! Convert from a Qt color.
        ftk::Color4F fromQt(const QColor&);

        //! Set whether the window is floating on top.
        void setFloatOnTop(bool, QWidget*);
    }
}
